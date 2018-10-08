#ifndef LOGGING_H
#define LOGGING_H

#define LOG_ERROR(fileptr, filename, fomat, ...) \
	fopen_s(&fileptr, filename, "a+"); \
	fprintf_s(fileptr, fomat, __VA_ARGS__); \
	fclose(fileptr);

#endif // !LOGGING_H

