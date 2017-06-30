/*
 * MALHEUR - Automatic Analysis of Malware Behavior
 * Copyright (c) 2009-2015 Konrad Rieck (konrad@mlsec.org)
 * University of Goettingen, Berlin Institute of Technology
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details.
 * --
 */

#include "config.h"
#include "malheur.h"
#include "common.h"
#include "mconfig.h"
#include "ftable.h"
#include "fmath.h"
#include "proto.h"
#include "util.h"
#include "cluster.h"
#include "class.h"
#include "export.h"

/* Global variables */
int verbose = 0;
config_t cfg;

static int print_conf = FALSE;
static char **input_files = NULL;
static int input_len = 0;
static int reset = FALSE;
static int save = TRUE;
static char *fvec_dump = NULL;
static malheur_action_t action = PROTOTYPE;
static malheur_state_t mstate;

/* Option string */
#define OPTSTRING       "no:c:s:hvVDC"

/**
 * Array of options of getopt_long()
 */
static struct option longopts[] = {
    {"reset", 0, NULL, 1001},
    {"dry", 0, NULL, 'n'},
    {"verbose", 0, NULL, 'v'},
    {"version", 0, NULL, 'V'},
    {"help", 0, NULL, 'h'},
    {"fvec_dump", 1, NULL, 1002},
    {"print_config", 0, NULL, 'C'},
    {"print_defaults", 0, NULL, 'D'},
    {"help", 0, NULL, 'h'},
    {"output_file", 1, NULL, 'o'},
    {"state_dir", 1, NULL, 's'},
    {NULL}
};

/**
 * Print usage of command line tool
 * @param argc Number of arguments
 * @param argv Argument values
 */
static void print_usage(void)
{
    printf("Usage: malheur [options] <action> <dataset>\n"
           "\nActions:\n"
           "  distance       Compute distance matrix for malware reports\n"
           "  prototype      Extract prototypes from malware reports\n"
           "  protodist      Compute distance matrix for prototypes\n"
           "  cluster        Cluster malware reports into similar groups\n"
           "  classify       Classify malware reports using labeled prototypes\n"
           "  increment      Incremental analysis of malware reports\n"
           "  info           Print information about internal state of Malheur\n"
           "\nOptions:\n"
           "  -s,  --state_dir <dir>       Set directory for internal state.\n"
           "  -o,  --output_file <file>    Set output file for analysis results.\n"
           "  -c,  --config_file <file>    Set configuration file.\n"
           "  -n,  --dry                   Dry run. Don't change internal state.\n"
           "       --reset                 Reset internal state of Malheur.\n"
           "       --fvec_dump <file>      Dump feature vectores in LibSVM format.\n"
           "  -C,  --print_config          Print the current configuration.\n"
           "  -D,  --print_defaults        Print the default configuration.\n"
           "  -v,  --verbose               Increase verbosity.\n"
           "  -V,  --version               Print version and copyright.\n"
           "  -h,  --help                  Print this help screen.\n"
           "\nSee manual page malheur(1) for more information.\n");
}

/**
 * Print configuration
 * @param msg Text to add to output
 */
static void print_config(char *msg)
{
    malheur_version(stderr);
    fprintf(stderr, "# ---\n# %s\n", msg);
    config_fprint(stderr, &cfg);
}


/**
 * Parse command line options
 * @param argc Number of arguments
 * @param argv Argument values
 */
static void malheur_parse_options(int argc, char **argv)
{
    int ch;

    /* reset getopt */
    optind = 0;

    while ((ch = getopt_long(argc, argv, OPTSTRING, longopts, NULL)) != -1) {
        switch (ch) {
        case 'c':
            /* Empty. See malheur_load_config() */
            break;
        case 'n':
            save = FALSE;
            break;
        case 1001:
            reset = TRUE;
            break;
        case 'v':
            verbose++;
            break;
        case 's':
            config_set_string(&cfg, "generic.state_dir", optarg);
            break;
        case 'o':
            config_set_string(&cfg, "generic.output_file", optarg);
            break;
        case 1002:
            fvec_dump = optarg;
            break;
        case 'V':
            malheur_version(stdout);
            exit(EXIT_SUCCESS);
            break;
        case 'D':
            print_config("Default configuration");
            exit(EXIT_SUCCESS);
            break;
        case 'C':
            print_conf = TRUE;
            break;
        case 'h':
        case '?':
            print_usage();
            exit(EXIT_SUCCESS);
            break;
        }
    }

    /* Check configuration */
    if (!config_check(&cfg)) {
        exit(EXIT_FAILURE);
    }

    /* We are through with parsing. Print the config if requested */
    if (print_conf) {
        print_config("Current configuration");
        exit(EXIT_SUCCESS);
    }

    argc -= optind;
    argv += optind;

    if (argc < 1)
        fatal("the <action> argument is required");

    /* Argument: action */
    if (!strcasecmp(argv[0], "prototype")) {
        action = PROTOTYPE;
    } else if (!strcasecmp(argv[0], "distance")) {
        action = DISTANCE;
    } else if (!strcasecmp(argv[0], "cluster")) {
        action = CLUSTER;
    } else if (!strcasecmp(argv[0], "classify")) {
        action = CLASSIFY;
    } else if (!strcasecmp(argv[0], "increment")) {
        action = INCREMENT;
    } else if (!strcasecmp(argv[0], "protodist")) {
        action = PROTODIST;
    } else if (!strcasecmp(argv[0], "info")) {
        action = INFO;
    } else {
        fatal("Unknown analysis action '%s'", argv[0]);
    }


    if (argc < 2 && action != PROTODIST && action != INFO)
        fatal("the <dataset> argument is required");

    /* Assign input files */
    input_files = argv + 1;
    input_len = argc - 1;
}

/**
 * Load configuration
 * @param argc Number of arguments
 * @param argv Argument values
 */
static void malheur_load_config(int argc, char **argv)
{
    char *cfg_file = NULL;
    int ch;

    /* Check for config file in command line */
    while ((ch = getopt_long(argc, argv, OPTSTRING, longopts, NULL)) != -1) {
        switch (ch) {
        case 'c':
            cfg_file = optarg;
            break;
        case 'h':
        case '?':
            print_usage();
            exit(EXIT_SUCCESS);
            break;
        default:
            /* empty */
            break;
        }
    }

    /* Init and load configuration */
    config_init(&cfg);

    if (cfg_file != NULL) {
        if (config_read_file(&cfg, cfg_file) != CONFIG_TRUE)
            fatal("Could not read configuration (%s in line %d)",
                  config_error_text(&cfg), config_error_line(&cfg));
    }

    /* Check configuration and set defaults */
    if (!config_check(&cfg)) {
        exit(EXIT_FAILURE);
    }

}


/**
 * Initialize malheur tool
 */
static void malheur_init()
{
    const char *state_dir;
    double shared;

    /* Init feature lookup table */
    config_lookup_float(&cfg, "cluster.shared_ngrams", &shared);
    if (shared > 0.0) {
        ftable_init();
    }

    /* Reset internal state */
    memset(&mstate, 0, sizeof(mstate));
    config_lookup_string(&cfg, "generic.state_dir", &state_dir);

    /* Create directory if not available */
    if (access(state_dir, W_OK)) {
        if (verbose > 0)
            printf("Creating state directory '%s'\n", state_dir);
        mkdir(state_dir, 0700);
    }

    /* Print configuration */
    if (verbose > 1)
        config_print(&cfg);
}


/**
 * Loads data from archives/directories into feature vectors
 * @return array of feature vectors
 */
static farray_t *malheur_load()
{
    farray_t *fa = NULL;
    for (int i = 0; i < input_len; i++) {
        /* Argument: Input */
        if (access(input_files[i], R_OK)) {
            warning("Could not access '%s'", input_files[i]);
            continue;
        }

        farray_t *f = farray_extract(input_files[i]);
        if (f) {
            fa = farray_merge(fa, f);
        }
    }

    if (!fa)
        fatal("No data available.");

    /* Dump feature vectors to file */
    if (fvec_dump)
        farray_save_libsvm_file(fa, fvec_dump);

    return fa;
}

/**
 * Saves the internal Malheur state. The state is used during incremental
 * analysis to distinguig clusters obtained during different runs
 * @param run Current run of analysis
 * @param proto Number of prototypes
 * @param rej Number of rejected reports
 */
static void malheur_save_state()
{
    FILE *f;
    const char *state_dir;
    char state_file[512];

    config_lookup_string(&cfg, "generic.state_dir", &state_dir);
    snprintf(state_file, 512, "%s/%s", state_dir, STATE_FILE);

    if (verbose > 0)
        printf("Saving internal state to '%s'.\n", state_file);

    f = fopen(state_file, "w");
    if (!f) {
        error("Could not open state file '%s'.", state_file);
        return;
    }

    fprintf(f, "run = %u\nprototypes = %u\nrejected = %u\n",
            mstate.run, mstate.num_proto, mstate.num_reject);

    fclose(f);
}

/**
 * Loads the internal Malheur state. The state is used during incremental
 * analysis to distinguig clusters obtained during different runs
 */
static int malheur_load_state()
{
    const char *state_dir;
    char state_file[512];
    FILE *f;
    int ret;

    config_lookup_string(&cfg, "generic.state_dir", &state_dir);
    snprintf(state_file, 512, "%s/%s", state_dir, STATE_FILE);

    if (access(state_file, R_OK))
        return FALSE;

    if (verbose > 0)
        printf("Loading internal state to '%s'.\n", state_file);

    f = fopen(state_file, "r");
    if (!f) {
        error("Could not open state file '%s'.", state_file);
        return FALSE;
    }

    ret = fscanf(f, "run = %u\nprototypes = %u\nrejected = %u\n",
                 &mstate.run, &mstate.num_proto, &mstate.num_reject);

    if (ret != 3) {
        error("Could not parse state file '%s'.", state_file);
        return FALSE;
    }

    fclose(f);
    return TRUE;
}

/**
 * Determines prototypes for the given malware reports
 */
static void malheur_prototype()
{
    assign_t *as;
    farray_t *fa, *pr;
    const char *state_dir, *output_file;
    char proto_file[512];

    config_lookup_string(&cfg, "generic.output_file", &output_file);
    config_lookup_string(&cfg, "generic.state_dir", &state_dir);
    snprintf(proto_file, 512, "%s/%s", state_dir, PROTO_FILE);

    /* Load data */
    fa = malheur_load();

    /* Extract prototypes */
    pr = proto_extract(fa, &as);
    if (verbose > 1)
        farray_print(pr);

    /* Save prototypes */
    if (save)
        farray_save_file(pr, proto_file);

    /* Export prototypes */
    export_proto(pr, fa, as, output_file);

    /* Clean up */
    assign_destroy(as);
    farray_destroy(pr);
    farray_destroy(fa);
}

/**
 * Clusters the given malware reports
 */
static void malheur_cluster()
{
    assign_t *as;
    farray_t *fa, *pr, *pn, *re;
    const char *state_dir, *output_file;
    char proto_file[512], reject_file[512];

    config_lookup_string(&cfg, "generic.output_file", &output_file);
    config_lookup_string(&cfg, "generic.state_dir", &state_dir);
    snprintf(proto_file, 512, "%s/%s", state_dir, PROTO_FILE);
    snprintf(reject_file, 512, "%s/%s", state_dir, REJECT_FILE);

    /* Load data */
    fa = malheur_load();

    /* Extract prototypes */
    pr = proto_extract(fa, &as);

    /* Cluster prototypes and extrapolate */
    cluster_t *c = cluster_linkage(pr, 0);
    cluster_extrapolate(c, as);
    cluster_trim(c);

    /* Save prototypes */
    pn = cluster_get_prototypes(c, as, pr);
    if (save)
        farray_save_file(pn, proto_file);
    farray_destroy(pn);

    /* Save rejected feature vectors */
    re = cluster_get_rejected(c, fa);
    if (save)
        farray_save_file(re, reject_file);
    farray_destroy(re);

    /* Export clustering */
    export_cluster(c, pr, fa, as, output_file);

    /* Export shared n-grams */
    export_shared_ngrams(c, fa, output_file);

    /* Clean up */
    cluster_destroy(c);
    assign_destroy(as);
    farray_destroy(pr);
    farray_destroy(fa);
}

/**
 * Classify the given malware reports
 */
static void malheur_classify()
{
    assign_t *as;
    farray_t *fa, *pr, *re;
    const char *state_dir, *output_file;
    char proto_file[512], reject_file[512];

    config_lookup_string(&cfg, "generic.output_file", &output_file);
    config_lookup_string(&cfg, "generic.state_dir", &state_dir);
    snprintf(proto_file, 512, "%s/%s", state_dir, PROTO_FILE);
    snprintf(reject_file, 512, "%s/%s", state_dir, REJECT_FILE);

    /* Check for prototype file */
    if (access(proto_file, R_OK))
        fatal("No prototype file for classifcation available");

    /* Load data */
    fa = malheur_load();

    /* Load prototypes */
    pr = farray_load_file(proto_file);

    /* Apply classification */
    as = class_assign(fa, pr);

    /* Save rejected feature vectors */
    re = class_get_rejected(as, fa);
    if (save)
        farray_save_file(re, reject_file);
    farray_destroy(re);

    /* Export classification */
    export_class(pr, fa, as, output_file);

    /* Clean up */
    assign_destroy(as);
    farray_destroy(pr);
    farray_destroy(fa);
}

/**
 * Classify the given malware reports
 */
static void malheur_increment()
{
    farray_t *pr = NULL, *tmp, *pn, *re;
    assign_t *as;
    const char *state_dir, *output_file;
    char proto_file[512], reject_file[512];

    config_lookup_string(&cfg, "generic.output_file", &output_file);
    config_lookup_string(&cfg, "generic.state_dir", &state_dir);
    snprintf(proto_file, 512, "%s/%s", state_dir, PROTO_FILE);
    snprintf(reject_file, 512, "%s/%s", state_dir, REJECT_FILE);

    /* Load internal state */
    malheur_load_state();

    /* Load data including rejected stuff */
    farray_t *fa = malheur_load();
    if (!access(reject_file, F_OK)) {
        tmp = farray_load_file(reject_file);
        fa = farray_merge(fa, tmp);
    }

    /* Classification */
    if (!access(proto_file, R_OK)) {
        pr = farray_load_file(proto_file);

        /* Apply classification */
        as = class_assign(fa, pr);
        tmp = class_get_rejected(as, fa);

        /* Export results */
        export_increment1(pr, fa, as, output_file);

        /* Clean up */
        farray_destroy(fa);
        farray_destroy(pr);
        assign_destroy(as);
        fa = tmp;
    } else {
        /* Export results */
        export_increment1(pr, fa, as, output_file);
    }

    /* Extract prototypes */
    pr = proto_extract(fa, &as);

    /* Cluster prototypes and extrapolate */
    cluster_t *c = cluster_linkage(pr, mstate.run + 1);
    cluster_extrapolate(c, as);
    cluster_trim(c);

    /* Save prototypes vectors */
    pn = cluster_get_prototypes(c, as, pr);
    if (save)
        farray_append_file(pn, proto_file);

    /* Save rejeted feature vectors */
    re = cluster_get_rejected(c, fa);
    if (save)
        farray_save_file(re, reject_file);

    /* Update state */
    mstate.run++;
    mstate.num_proto = pn->len;
    mstate.num_reject = re->len;

    /* Save state */
    if (save)
        malheur_save_state();

    /* Export results */
    export_increment2(c, pr, fa, as, output_file);

    /* Clean up */
    cluster_destroy(c);
    assign_destroy(as);

    farray_destroy(re);
    farray_destroy(pn);
    farray_destroy(pr);
    farray_destroy(fa);
}

/**
 * Display information about internal state of Malheur
 */
static void malheur_info()
{
    const char *state_dir;
    char state_file[512];

    config_lookup_string(&cfg, "generic.state_dir", &state_dir);
    snprintf(state_file, 512, "%s/%s", state_dir, STATE_FILE);

    /* Load internal state */
    if (!malheur_load_state()) {
        printf("No internal state stored in %s\n", state_dir);
        return;
    }

    printf("Internal state from %s\n", state_file);
    printf("       Malheur run: %u\n", mstate.run);
    printf(" Stored prototypes: %u\n", mstate.num_proto);
    printf("  Rejected reports: %u\n", mstate.num_reject);

}

/**
 * Computes a distance matrix and saves the result to a file
 */
static void malheur_distance()
{
    const char *output_file;
    config_lookup_string(&cfg, "generic.output_file", &output_file);

    /* Load data */
    farray_t *fa = malheur_load();

    /* Allocate distance matrix */
    double *d = malloc(fa->len * fa->len * sizeof(double));
    if (!d)
        fatal("Could not allocate similarity matrix");

    /* Compute distance matrix */
    farray_dist(fa, fa, d);

    /* Save distance matrix */
    export_dist(d, fa, output_file);

    /* Clean up */
    free(d);
    farray_destroy(fa);
}

/**
 * Computes a distance matrix for the prototypes and saves the result to a file
 */
static void malheur_protodist()
{
    farray_t *pr;
    const char *state_dir, *output_file;
    char proto_file[512];

    config_lookup_string(&cfg, "generic.output_file", &output_file);
    config_lookup_string(&cfg, "generic.state_dir", &state_dir);
    snprintf(proto_file, 512, "%s/%s", state_dir, PROTO_FILE);

    /* Check for prototype file */
    if (access(proto_file, R_OK))
        fatal("No prototype file for classifcation available");

    /* Load prototypes */
    pr = farray_load_file(proto_file);
    if (verbose > 1)
        farray_print(pr);

    /* Allocate distance matrix */
    double *d = malloc(pr->len * pr->len * sizeof(double));
    if (!d)
        fatal("Could not allocate similarity matrix");

    /* Compute distance matrix */
    farray_dist(pr, pr, d);

    /* Save distance matrix */
    export_dist(d, pr, output_file);

    /* Clean up */
    free(d);
    farray_destroy(pr);
}

/**
 * Exits the malheur tool.
 */
static void malheur_exit()
{
    /* Destroy feature lookup table */
    if (verbose > 0)
        ftable_print();
    ftable_destroy();

    /* Destroy configuration */
    config_destroy(&cfg);
}

/**
 * Main function of Malheur
 * @param argc Number of arguments
 * @param argv Argument values
 * @return Exit code
 */
int main(int argc, char **argv)
{

    malheur_load_config(argc, argv);
    malheur_parse_options(argc, argv);
    malheur_init();

    /* Perform action */
    switch (action) {
    case DISTANCE:
        malheur_distance();
        break;
    case PROTOTYPE:
        malheur_prototype();
        break;
    case CLUSTER:
        malheur_cluster();
        break;
    case CLASSIFY:
        malheur_classify();
        break;
    case INCREMENT:
        malheur_increment();
        break;
    case PROTODIST:
        malheur_protodist();
        break;
    case INFO:
        malheur_info();
    }

    malheur_exit();
    return EXIT_SUCCESS;
}
