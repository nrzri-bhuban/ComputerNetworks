#include <stdio.h>
#include <stdlib.h>

int main()
{
	FILE *fp;
	// Command to capture 5 packets on interface h1-eth0 and write them to a file.
	// The '-w' flag tells TShark to write the raw packet data to a file.

	const char *tshark_command = "tshark -i h1-eth0 -c 1000 -w /tmp/As1-a.pcapng";
	
	printf("Executing command: %s\n", tshark_command);
	printf("Capturing packets and writing to output.pcapng...\n");

	// We use popen to execute the command. We don't need to read its output
	// because the -w flag handles file writing directly.

	fp = popen(tshark_command, "r");

	if (fp == NULL) {
		perror("Failed to run command");
		return 1;
	}

	// pclose waits for the command to terminate and closes the stream.
	int status = pclose(fp);

	if (status == -1) {
		perror("pclose failed");
		return 1;
	} else {
		printf("Packet capture finished.\n");
		printf("The output has been saved to 'As1-a.pcapng'.\n");
	}
	return 0;
}
