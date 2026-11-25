#include "uri_target.h"

int parsing_for_origin_form(struct http_uri_target_parser *uri_parser_p, char **cursor) {
    if (uri_parser_p->state == UT_START && **cursor == '/') {
        uri_parser_p->start_p = (*cursor);
        (*cursor)++;
        uri_parser_p->state = UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL;
        return 1;
    } else if (uri_parser_p->state >= UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL && uri_parser_p->state <= UT_PARSE_FOR_ORIGIN_QUERY) {
        return 1;
    }
    return 0;
}

void init_target_struct_for_origin_target(struct request_line_uri_target *uri_target_p) {
    uri_target_p->uri_target.origin_form.absolute_path = "";
    uri_target_p->uri_target.origin_form.query = NULL;
}


void parse_for_origin(struct request_line_uri_target *uri_target_p, struct http_uri_target_parser *uri_parser_p, char **cursor, char *end) {
    char *val;
    init_target_struct_for_origin_target(uri_target_p);
    // var made for inspecting value with debugger
    int still_parsing;

    while ((still_parsing = (*cursor <= end)) == 1) {
        if (**cursor == '#') {
            uri_parser_p->state = INVALID_URI_TARGET;
            uri_target_p->form = URI_FORM_NOT_SET;
            return;
        } else if (uri_parser_p->state >= UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL && uri_parser_p->state <=UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_2) {
                if (**cursor == '?') {
                    val = cpy_str(uri_parser_p->start_p, *cursor);
                    if (val == NULL) {
                        print_server_error("malloc error in cpy_str");
                        uri_parser_p->state = INVALID_URI_TARGET;
                        return;
                    }
                    uri_target_p->uri_target.origin_form.absolute_path = val;
                    uri_parser_p->state = UT_PARSE_FOR_ORIGIN_QUERY;
                    uri_parser_p->start_p = *cursor;
                } else if (**cursor == ' ') {
                    // handle no query
                    if (uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL) {
                        val = cpy_str(uri_parser_p->start_p, *cursor);
                        // printf("cursor == space and uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL");
                        if (val == NULL) {
                            print_server_error("malloc error in cpy_str");
                            uri_parser_p->state = INVALID_URI_TARGET;
                            return;
                        }
                        uri_target_p->uri_target.origin_form.absolute_path = val;
                        uri_target_p->form = ORIGIN_FORM;
                        uri_parser_p->state = VALID_URI_TARGET;
                    } else {
                        uri_parser_p->state = INVALID_URI_TARGET;
                        uri_target_p->form = URI_FORM_NOT_SET;
                    }
                    return;
                } else if (**cursor == '%') {
                    if (uri_parser_p->state != UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL) {
                        uri_parser_p->state = INVALID_URI_TARGET;
                        uri_target_p->form = URI_FORM_NOT_SET;
                        return;
                    }
                    uri_parser_p->state = UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_1;
                } else if (is_pchar(**cursor) || **cursor == '/') {
                    if (uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_1 || uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_2) {
                        if (!is_hex_digit(**cursor)) {
                            uri_parser_p->state = INVALID_URI_TARGET;
                            uri_target_p->form = URI_FORM_NOT_SET;
                            return;
                        }
                        if (uri_parser_p->state == UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_1)
                        uri_parser_p->state = UT_PARSE_FOR_ORIGIN_ABS_PATH_EXPT_PCT_HEX_2;
                        else
                        uri_parser_p->state = UT_PARSE_FOR_ORIGIN_ABS_PATH_NORMAL;
                    }
                } else {
                    uri_parser_p->state = INVALID_URI_TARGET;
                    uri_target_p->form = URI_FORM_NOT_SET;
                    return;
                }
        } else if (uri_parser_p->state == UT_PARSE_FOR_ORIGIN_QUERY) {
            if (**cursor == ' ') {
                val = cpy_str(uri_parser_p->start_p, *cursor);
                if (val == NULL) {
                    print_server_error("malloc error in cpy_str");
                    uri_parser_p->state = INVALID_URI_TARGET;
                    return;
                }
                uri_target_p->uri_target.origin_form.query = val;
                uri_target_p->form = ORIGIN_FORM;
                uri_parser_p->state = VALID_URI_TARGET;
                uri_parser_p->start_p = *cursor;
                return;
            }
        }
        (*cursor)++;
        }
    }
