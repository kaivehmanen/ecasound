/*   
    This is ecalength, a few lines of code pompously named so because they  
    let one retrieve the length of an audio file from the command line  
    using ecasound's engine.  

    Limitations:  
    - With files without header information (raw files), ecalength will only work 
      correctly if the audio file is at a sampling rate of 44100 hz.
      (Addressed with the -a switch.)
    - It is not foolproof, feeding it with something other than an audio  
      file WILL result in ugly things being spewed back.  
      (A bit better)
    - A thousand more that I haven't thought of.  

    Please post back any improvement you make; I can be reached at:  
    observer@colba.net  

    note: Compile it with:  
    gcc -Wall -lecasoundc -o ecalength ecalength.c  

    last updated: Thu May 10 15:56:18 EDT 2001
- Now works with the new ai/ao scheme.
- Switches implemented, made suitable for scripting.
- Format querying/setting.
- Better error handling.  
*/ 

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include "ecasoundc.h" 

#define FALSE          0 
#define TRUE           1 

void make_human(int length, unsigned int *min, unsigned char *sec); 
void print_usage(char* name); 

struct options { 
  char adjust;
  char format; 
  char total; 
  char script; 
  char human;
  char bits;
  char ccount; 
  char rate;
}; 

int main(int argc, char *argv[]) { 
  char cmd[500], fstring[16], status = 0, curopt, *optstr = "ftsmhbcra:"; 
  unsigned char sec; 
  float curfilelength, totlength = 0; 
  unsigned int min, curarg; 
  FILE *file; 
  struct options opts; 

  opts.adjust = FALSE;
  opts.format = FALSE; 
  opts.total = FALSE; 
  opts.script = FALSE; 
  opts.human = FALSE; 
  opts.bits = FALSE;
  opts.ccount = FALSE;
  opts.rate = FALSE;

  while ((curopt = getopt(argc, argv, optstr)) != -1) { 
    switch (curopt) { 
    case 'a' : opts.adjust = TRUE;
      strcpy(fstring, optarg);
      break;
    case 'f' : opts.format = TRUE; 
      break; 
    case 't' : opts.total = TRUE; 
      break; 
    case 's' : opts.script = TRUE; 
      break; 
    case 'm' : opts.human = TRUE; 
      break; 
    case 'b' : opts.bits = TRUE;
      break;
    case 'c' : opts.ccount = TRUE;
      break;
    case 'r' : opts.rate = TRUE;
      break;
    case 'h' : print_usage(argv[0]);
      exit(0);
    case '?' : print_usage(argv[0]);
      exit(1);
    } 
  } 

  if (argc-optind == 0) {
    print_usage(argv[0]);
    exit(1);
  }

  if ((opts.script) && (((opts.format) && (opts.human)) ||
			((opts.format) && (((opts.bits) && ((opts.ccount) || 
							    (opts.rate))) ||
					   ((opts.ccount) && (opts.rate)))))) {
    fprintf(stderr, "Error: In script mode not more than one further mode can be specified.\n");
    print_usage(argv[0]);
    exit(1);
  }

  eci_init(); 
  eci_command("cs-add main"); 
  eci_command("c-add main"); 
  eci_command("ao-add null"); 
  if (opts.adjust) {
    if (strncmp(":", fstring, 1) == 0) { sprintf(cmd, "cs-set-audio-format %s", fstring+1); }
    else { sprintf(cmd, "cs-set-audio-format %s", fstring); }
    eci_command(cmd);
    if (strlen(eci_last_error()) != 0) {
      fprintf(stderr, "Argument to -a is badly formatted.\n");
      print_usage(argv[0]);
      exit(1);
    }
  }

  curarg = optind; 

  while(curarg < argc) { 
    if ((file = fopen(argv[curarg], "r")) != NULL) { 
      fclose(file); 
      sprintf(cmd, "ai-add %s", argv[curarg]); 
      eci_command(cmd); 
      eci_command("cs-connect"); 
      if (strlen(eci_last_error()) == 0) {
	sprintf(cmd, "ai-select %s", argv[curarg]); 
	eci_command(cmd); 
	eci_command("ai-get-length"); 
	curfilelength = eci_last_float(); 
	if (opts.format) { 
	  eci_command("ai-get-format"); 
	  strcpy(fstring, eci_last_string()); 
	} 
	eci_command("cs-disconnect"); 
	eci_command("ai-remove"); 
	if (!(opts.script) || ((opts.script && opts.human))) { 
	  make_human((int)(curfilelength+0.5), &min, &sec); 
	} 
	if (!(opts.script)) { printf("%s: ", argv[curarg]); } 
	if (!(opts.script) ||  
	    ((opts.script) && (!(opts.format) && !(opts.human)))) { 
	  printf("%.3f", curfilelength); 
	} 
	if (!(opts.script)) { printf("s   \t("); } 
	if (!(opts.script) || ((opts.script) && (opts.human))) { 
	  printf("%im%is", min, sec); 
	} 
	if (!(opts.script)) { printf(")"); } 
	if ((opts.format) && 
	    !((opts.format) && ((opts.bits) || (opts.ccount) || (opts.rate)))) { 
	  if (!(opts.script)) { printf("   \t"); } 
	  printf("%s", fstring); 
	} 

	if ((opts.format) && (opts.script) && (opts.bits)) { 
	  printf("%s", strtok(fstring+1, "_")); 
	}

	if ((opts.script) && (opts.format) && (opts.ccount)) {
	  strtok(fstring, ",");
	  printf("%s", strtok(NULL, ","));
	}

	if ((opts.format) && (opts.script) && (opts.rate)) {
	  strtok(fstring, ",");
	  strtok(NULL, ",");
	  printf("%s", strtok(NULL, ","));
	}

	printf("\n"); 
	if ((opts.total) && !(opts.script)) { 
	  totlength += curfilelength; 
	} 
      }
	  else {
	    if (opts.script) { printf("-2\n"); }
	    else { printf("%s: Read error.\n", argv[curarg]); }
	    status = -2;
	    eci_command("ai-remove");
	  }
    } 
    else { 
      if (opts.script) { printf("-1\n"); }
      else { printf("%s: fopen error.\n", argv[curarg]); }
      status = -1;
    } 
    curarg++; 
  } 

  if ((opts.total) && !(opts.script)) { 
    make_human((int)(totlength+0.5), &min, &sec); 
    printf("Total: %.3fs \t\t(%im%is)\n", totlength, min, sec); 
  } 

  eci_command("cs-remove");
  eci_cleanup();

  exit(status); 
} 

void make_human(int length, unsigned int *min, unsigned char *sec) { 
  *min = (length/60); 
  *sec = (length % 60); 
} 

void print_usage(char *name) { 
  printf("Usage: %s [-ahtsfmbcr] FILE1 [FILE2] [FILEn]\n 
       -h      Prints this usage message.  (help)
       -a[:]bits,channels,rate     Changes the format assumed by default 
                                   for headerless data.  (adjust)
       -t      Prints the summed length of all the files processed.  (total)
                 (Ignored if with -s) 
       -s      Enables script mode: One info type per file per line.   (script)
                 (Defaults to length in secs.) 
       -f      With -s will return the format string as info, alone it will 
               add it to the main display.  (format)
           -b  If -s and -f are enabled with this the info printed will be 
              the sample's bitwidth.  (bits)
           -c  If -s and -f are enabled with this the info printed will be 
               the channel count.  (channel count)
           -r  If -s and -f are enabled with this the info printed will be 
               the sampling rate.  (rate)
       -m      Will print human computable time as in main display but in 
               batch fashion.  (minutes)
                 (Only with -s)
   (Note that out of context options will be silently ignored.)\n\n",
	 name);
}
