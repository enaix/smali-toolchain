#include <getopt.h>
#include <signal.h>

#include "replace.h"



void log_open_error()
{
	printf("open() failed : %s\n", strerror(errno));
	#ifdef ENABLE_DEBUG
	raise(SIGTRAP);
	#endif
}


#define INJECT_PATH_MAX 256

int output_logic(char* output, int* in_fd, int* out_fd)
{
	assert(strlen(output) + 16 <= INJECT_PATH_MAX && "Path is too long to be processed. Please increase INJECT_PATH_MAX");

	char fname[INJECT_PATH_MAX];
	// Rename template (.smali.template -> .smali)
	char* tok = strtok(output, ".");
	char* prev = NULL;
	strcat(fname, tok); // template cannot be the start

	while((tok = strtok(NULL, ".")) != NULL)
	{
		if (prev != NULL)
		{
			strcat(fname, ".");
			strcat(fname, prev);
		}
		
		prev = tok;
	}
	// last operation
	if (strcmp(prev, "template") != 0)
	{
		// the file does not end with .template and we have to move the existing file to .template.bak
		char new_fname[INJECT_PATH_MAX];
		// add the last token, since it should be "smali"
		strcat(fname, ".");
		strcat(fname, prev);
		
		strcpy(new_fname, fname);
		strcat(new_fname, ".template.bak");

		#ifdef ENABLE_DEBUG
		printf("opening (%s -> %s) for reading, %s for writing...\n", fname, new_fname, fname);
		#endif

		if (rename(fname, new_fname) == -1)
		{
			log_open_error();
			return 0;
		}

		*in_fd = open(new_fname, O_RDONLY, 0640);
		if (*in_fd == -1)
		{
			log_open_error();
			return 0;
		}

		*out_fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, 0640);
		if (*out_fd == -1)
		{
			log_open_error();
			close_fd(*in_fd, *out_fd, -1);
			return 0;
		}
	} else {
		// Output .smali file
		#ifdef ENABLE_DEBUG
		printf("opening %s for writing", fname);
		#endif
		*out_fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, 0640);
		if (*out_fd == -1)
		{
			printf("...\n");
			log_open_error();
			return 0;
		}
		strcat(fname, ".template");
		#ifdef ENABLE_DEBUG
		printf(", %s for reading...\n", fname);
		#endif
		*in_fd = open(fname, O_RDONLY, 0640);
		if (*in_fd == -1)
		{
			log_open_error();
			close_fd(*in_fd, *out_fd, -1);
			return 0;
		}
	}
	
	return 1;
}


void print_help()
{
	printf(
		"Usage: inject [OPTION]... [COMMAND] [FILE]\n"
		"Inject smali payload code into the FILE template. If INPUT is not given, reads code from STDIN.\n\n"
		"COMMAND specifies where to inject the code:\n"
		"  locals         \tset the number of local variables, can be injected only once\n"
		"  init           \tinject the code to the main class constructor, can be done multiple times\n"
		"  methods        \tinject new methods to the main class, can be done multiple ones\n\n"
		"OPTION may be the following:\n"
		"  -h, --help     \tprint this message and exit\n"
		"  -f, --file=INPUT\tinject the code from INPUT\n"
	);
}


int main(int argc, char** argv)
{
	static struct option opts[] = {
		{"help", no_argument, NULL, 'h'},
		{"file", required_argument, NULL, 'f'},
		{NULL, 0, NULL, 0}
	};
	int opt = 0;
	char* subst_file = NULL;

	while ((opt = getopt_long(argc, argv, "hf:", opts, NULL)) != -1)
	{
		switch (opt)
		{
		case 'h':
			print_help();
			return 0;
		case 'f':
			subst_file = optarg;
			break;
		case '?':
			printf("Argument not recognized: %s\n", argv[optind]);
			print_help();
		}
	}

	const char* cmd = NULL;
	char* output = NULL;

	if (optind < argc)
	{
		cmd = argv[optind];
		if (optind + 1 < argc)
			output = argv[optind + 1];
		// Other arguments
		if (optind + 2 < argc)
		{
			for (int i = optind + 3; i < argc; i++)
				printf("Argument not recognized: %s\n", argv[i]);
			print_help();
			return 1;
		}
	}

	// Parse output
	int in_fd, out_fd, subst_fd;
	if (!output)
	{
		print_help();
		return 1;
	}
	if (!output_logic(output, &in_fd, &out_fd))
	{
		log_open_error();
		return 1;
	}

	if (!subst_file)
	{
		subst_fd = 0; // stdin
	} else {
		subst_fd = open(subst_file, O_RDONLY, 0640);
		if (subst_fd == -1)
		{
			log_open_error();
			close_fd(in_fd, out_fd, -1);
			return 1;
		}
	}
	
	// handle command
	int ok;
	if (strcmp(cmd, "locals") == 0)
	{
		static char pattern[] = "__ENX_LOCALS_NUM__";
		ok = replace_single(in_fd, out_fd, subst_fd, pattern, sizeof(pattern) - 1, 0);
	}
	else if (strcmp(cmd, "init") == 0)
	{
		static char pattern[] = "#__ENX_CONSTRUCTOR_INIT__";
		ok = replace_single(in_fd, out_fd, subst_fd, pattern, sizeof(pattern) - 1, 1);
	}
	else if (strcmp(cmd, "methods") == 0)
	{
		static char pattern[] = "#__ENX_DEFAULT_OBJECT_METHODS__";
		ok = replace_single(in_fd, out_fd, subst_fd, pattern, sizeof(pattern) - 1, 1);
	} else {
		printf("Bad command: %s\n", cmd);
		print_help();
		close_fd(in_fd, out_fd, subst_fd);
		return 1;
	}
	close_fd(in_fd, out_fd, subst_fd);
	
	if (!ok)
	{
		printf("inject failed\n");
		return 1;
	}
	return 0;
}






