#include <stdio.h>
#include <ecasoundc.h>

/* compile with: "gcc -o jackdemo jackdemo.c -lecasoundc" */

int main(int argc, char *argv[])
{
  eci_init();

  /* create the chainsetup and one chain */
  eci_command("cs-add jackdemo_chainsetup");
  eci_command("c-add chain1");

  /* add the audio inputs and outputs */
  eci_command("ai-add foo.wav");
  eci_command("ao-add jack_alsa,out");

  /* add an LADSPA plate reverb */
  eci_command("cop-add -el:plate,50,0.5,0.5");

  /* select the 3rd param (wet/dry) for real-time control */
  eci_command("copp-select 3");

  /* connect the setup and start */
  eci_command("cs-connect");
  if (eci_error()) {
    printf("cs-connect error:\n%s\n\n", eci_last_error());
  }
  else {
    eci_command("start");

    while(1) {
      double curpos;
      sleep(1);
      /* fetch current play position */ 
      eci_command("get-position");
      curpos = eci_last_float();
      if (curpos > 10.0) {
	/* at pos=10sec, quit playing */
	break;
      }
      else if (curpos > 5.0) {
	/* at pos=5sec, set reverb length to 80% */
	eci_command_float_arg("copp-set", 0.8);
      }
    }

    eci_command("status");  
    printf("General Status::\n%s\n\n", eci_last_string());

    eci_command("aio-status");  
    printf("Audio Object Status:\n%s\n\n", eci_last_string());

    eci_command("stop");
    eci_command("cs-disconnect");
  }

  eci_cleanup();

  return(0);
}
