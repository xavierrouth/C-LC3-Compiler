#include <stdlib.h>
#include <stdio.h>
#include <argp.h>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "AST.h"
#include "codegen.h"
#include "analysis.h"
#include "error.h"

const char* argp_program_version = "1.0";
const char* argp_program_bug_address = "<xrouth2@illinois.edu>";

static char doc[] = "A C to LC3 Compiler built for students at the University of Illinois Urbana-Champaign by HKN at https://hkn-alpha.netlify.app/.";
static char args_doc[] = "file..."; 

static struct argp_option options[] = { 
    //{ "input", 'i', "FILE", 0, "Input path.", 0},
    { "output", 'o', "FILE", 0, "Output path", 0},
    { "verbose", 'v', 0, 0, "Produce verbose output", 0},
    { "debug", 'g', 0, 0, "Enable debugging information", 0}, // These two are just so compiler explorer doesn't complain.
    { "asm", 'S', 0, 0, "Enable assembly output", 0}, 
    //{ "print-ast", 0, 0},
    { 0 } 
};

struct arguments {
    char* input_path;
    int silent, verbose;
    char* output_path;
};

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
    //arguments.input_path = argv[argc-1]; // This seems like I could do it better.
    arguments.output_path = "out.asm";
    arguments.verbose = 0;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    
    if (argc == 1) {
        return 1;
    } 

    if ((f = fopen(arguments.input_path, "rb")) == NULL) {
        printf("Invalid input file path:\n");
        printf("%s\n", arguments.input_path);
        return 1;
    }

    set_out_file(arguments.output_path);

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char file_buffer[fsize + 1];
    fread(file_buffer, fsize, 1, f);
    fclose(f);

    file_buffer[fsize] = 0; //Set null terminator

    // Init Lexer
    init_lexer(file_buffer, fsize);

    //if (arguments.verbose) printf("Done Lexering\n");
    
    init_parser(file_buffer, fsize);
    //if (arguments.verbose) printf("Done Init Parser\n");
    build_ast();

    //if (arguments.verbose) printf("Done Building AST\n");

    ast_node_t root = get_root();    

    if (arguments.verbose) print_ast(root);
    analysis(root);
    
    print_errors();
    //if (arguments.verbose) printf("Beginning Code gen:\n");
    emit_ast(root);
    free_ast(root);

    close_out_file();
    
    return 0;
}
