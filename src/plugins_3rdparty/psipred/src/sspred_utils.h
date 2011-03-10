#ifndef SSPRED_UTILS_H
#define SSPRED_UTILS_H

class QByteArray;
class QTemporaryFile;

#define PSIPRED_ANNOTATION_NAME "psipred_results"

/* Dump error message */
void err(const char *s);
/* Dump error message and exit */
void fail(const char *s);
/* Convert AA letter to numeric code (0-20) */
int aanum(int ch);
/* generate matrix from sequence */
int seq2mtx(const char* seq, int length, const char* outFileName);
/* generate matrix from sequence */
int seq2mtx(const char* seq, int length, QTemporaryFile* tmpFile);




#endif
