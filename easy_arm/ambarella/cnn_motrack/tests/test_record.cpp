#include "record_stream.h"

static void sigstop(int)
{
	/// statistics_run = 0;
	/// dram_test_run = 0;
	write_video_file_run = 0;
}
int main(int argc, char **argv)
{
	int ret = 0;
	int err = 0;

	class recode_stream_ record_stream;

	// register signal handler for Ctrl+C,  Ctrl+'\'  ,  and "kill" sys cmd
	signal(SIGINT, sigstop);
	signal(SIGQUIT, sigstop);
	signal(SIGTERM, sigstop);

	if (record_stream.init_data() < 0)
	{
		fprintf(stderr, "data initiation failed!\n");
		ret = -1;
	}

	if (ret == 0)
	{
		err = record_stream.capture_encoded_video();
		if (err != 0)
		{
			fprintf(stderr, "capture encoded video failed: %s\n", strerror(err));
			ret = -1;
		}
	}

	record_stream.deinit_data();

	return ret;
}