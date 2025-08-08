#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

int check_date(long date)
{ // date is given as nanoseconds
	struct timespec current_time;
	int result = clock_gettime(CLOCK_REALTIME, &current_time);
	if (result == -1)
	{
		return 0;
	}
	long years_in_seconds_31 = 978307200; // the amount of seconds from the year 1970 to 2001
	long date_seconds = date / 1000000000;
	long seconds_from_message = current_time.tv_sec - years_in_seconds_31 - date_seconds;

	if (seconds_from_message > 8)
	{
		return 0;
	}
	return 1;
}

int callback(void *log, int column_count, char **colum_values, char **colum_names)
{
	(void)column_count;
	(void)colum_names;

	if (colum_values[0] == NULL)
	{
		return 0;
	}

	if (check_date(strtol(colum_values[1], NULL, 0)))
	{

		char *message = *colum_values;

		char verification_code[6 + 1];
		strncpy(verification_code, message + 22, 6);
		verification_code[6] = '\0';

		FILE *pipe = popen("pbcopy", "w");

		if (!pipe)
		{
			fprintf(log, "Failed to open pipe to copy verification code to the clipboard");
			fclose(log);
			return 1;
		}
		fprintf(pipe, "%s", verification_code);
		pclose(pipe);
	}

	return 0;
}

int main(void)
{

	const char *home = getenv("HOME");

	char db_path[PATH_MAX];
	sprintf(db_path, "%s/Library/messages/chat.db", home);

	char log_dir_path[PATH_MAX];
	sprintf(log_dir_path, "%s/Library/logs/Verification-code", home);

	if (mkdir(log_dir_path, 0755) != 0)
	{
		if (errno != EEXIST)
		{
			return 1;
		}
	}

	char log_file_path[PATH_MAX];
	sprintf(log_file_path, "%s/logs.txt", log_dir_path);

	FILE *log = fopen(log_file_path, "w");

	if (!log)
		return 1;

	fprintf(log, "The logs from the latest execution:\n\n");

	fprintf(log, "Database path: %s\n", db_path);

	sqlite3 *db;
	int open_response = sqlite3_open(db_path, &db);

	if (open_response != SQLITE_OK)
	{
		fprintf(log, "Error opening db connection: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		fclose(log);
		return 1;
	}
	char *error_pointer = NULL;

	while (1)
	{

		int exec_response = sqlite3_exec(
				db,
				"select text, date from message where handle_id = (select ROWID from handle where id = \"Microsoft\") order by date DESC limit 1;",
				callback,
				log,
				&error_pointer);

		if (exec_response != SQLITE_OK)
		{
			fprintf(log, "Error retrieving data: %s\n", error_pointer);
			sqlite3_free(error_pointer);
			sqlite3_close(db);
			fclose(log);
			return 1;
		}

		sleep(3);
	}

	sqlite3_close(db);

	fprintf(log, "\nFinnished, no errors!\n");

	fclose(log);
	return 0;
}
