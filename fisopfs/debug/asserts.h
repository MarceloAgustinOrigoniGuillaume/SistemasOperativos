#ifndef ASSERTS_H
#define ASSERTS_H


// ASSERTS RECIBE UNA VARIABLE DONDE GUARDAR EL RESULTADO!
#define ASSERT_EQ(given, a, b, msg)                                            \
	given = a;                                                             \
	if (given != b) {                                                      \
		fprintf(stderr, msg, given);                                   \
		fprintf(stderr, "\n");                                         \
		return -1;                                                     \
	}

#define ASSERT_EQ_STR(given, out, b, msg)                                      \
	given = out;                                                           \
	if (given == NULL || strcmp(given, b) != 0) {                          \
		fprintf(stderr, msg, given);                                   \
		fprintf(stderr, "\n");                                         \
		return -1;                                                     \
	}


#define ASSERT_TRUE(given, a, msg)                                             \
	given = a;                                                             \
	if (!given) {                                                          \
		fprintf(stderr, msg, given);                                   \
		fprintf(stderr, "\n");                                         \
		return -1;                                                     \
	}

#define ASSERT_FALSE(given, a, msg)                                            \
	given = a;                                                             \
	if (given) {                                                           \
		fprintf(stderr, msg, given);                                   \
		fprintf(stderr, "\n");                                         \
		return -1;                                                     \
	}


// EXPECT ASUME NO HAY PROBLEMA CON REEMPLAZAR MULTIPLES VECES!
#define EXPECT_EQ(given, b, msg)                                               \
	if (given != b) {                                                      \
		fprintf(stderr, msg, given);                                   \
		fprintf(stderr, "\n");                                         \
		return -1;                                                     \
	}

#define EXPECT_EQ_STR(given, b, msg)                                           \
	if (given == NULL || strcmp(given, b) != 0) {                          \
		fprintf(stderr, msg, given);                                   \
		fprintf(stderr, "\n");                                         \
		return -1;                                                     \
	}


#define EXPECT_TRUE(given, msg)                                                \
	if (!given) {                                                          \
		fprintf(stderr, msg, given);                                   \
		fprintf(stderr, "\n");                                         \
		return -1;                                                     \
	}

#define EXPECT_FALSE(given, msg)                                               \
	if (given) {                                                           \
		fprintf(stderr, msg, given);                                   \
		fprintf(stderr, "\n");                                         \
		return -1;                                                     \
	}


typedef int (*test_t)();
typedef int bool;

void addTest(test_t test);
int testCount();

int runTests(bool failFast);


int allocTests(int count);
void freeTests();

#endif
