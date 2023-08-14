#include <stdlib.h>
#include <stdio.h>
#include <argp.h>

#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/analysis.h"
#include "frontend/error.h"

#include "codegen/codegen.h"
#include "codegen/asmprinter.h"

const char* argp_program_version = "1.0";
const char* argp_program_bug_address = "<xrouth2@illinois.edu>";

static char doc[] = "A C to LC3 Compiler built for students at the University of Illinois Urbana-Champaign by HKN (https://hkn-alpha.netlify.app).";
static char args_doc[] = "file..."; 

static struct argp_option options[] = { 
    //{ "input", 'i', "FILE", 0, "Input path.", 0},
    { "output", 'o', "FILE", 0, "Output path", 0},
    { "verbose", 'v', 0, 0, "Produce verbose output", 0},
    { "debug", 'g', 0, 0, "Enable debugging information", 0}, // These two are just so compiler explorer doesn't complain.
    { "asm", 'S', 0, 0, "Enable assembly output", 0},
    { "global-data", 0, 0, 0, "Use global data pointer instead of data section", 0},
    { "sandbox", 0, 0, 0, "Disables most semantic analysis errors (type checking)", 0},
    //{ "print-ast", 0, 0},
    { 0 } 
};

struct arguments {
    char* input_path;
    int silent, verbose;
    char* output_path;
};

extern asm_block_t program_block;
extern parser_error_handler error_handler;

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
        case 'v': arguments->verbose = 1; break;
        //case 'i': arguments->input_path = arg; break;
        case 'o': arguments->output_path = arg; break;
        case 'g': break; // Doesn't do anything yet.
        case 'S': break;
        case ARGP_KEY_ARG: 
            if (state->arg_num >= 1)
                argp_usage(state);
            arguments->input_path = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1)
                argp_usage(state);
            break;
    default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0};

int main(int argc, char **argv) {
    FILE* f;

    struct arguments arguments;

    /* Default args:*/
    arguments.output_path = "out.asm";
    arguments.verbose = 0;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    if ((f = fopen(arguments.input_path, "rb")) == NULL) {
        printf("Invalid input file path:\n");
        printf("%s\n", arguments.input_path);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char file_buffer[fsize + 1];
    fread(file_buffer, fsize, 1, f);
    fclose(f);
 
    file_buffer[fsize] = EOF; // Set EOF

    // Init Lexer
    init_lexer(file_buffer, fsize);
    init_parser(file_buffer, fsize);

    build_ast();

    //if (arguments.verbose) printf("Done Building AST\n");

    ast_node_t root = get_root();    

    if (arguments.verbose) print_ast(root);
    analysis(root);
    
    print_errors();
    //if (arguments.verbose) printf("Beginning Code gen:\n");
    if (error_handler.num_errors != 0) {
        return 1;
    }
    emit_ast(root);

    write_to_file(arguments.output_path, &program_block);
    
    return 0;
}
