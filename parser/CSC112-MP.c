// CSC 112 JN1
// BOQUILON && ISIDERIO

#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

int main()
{

    FILE *file;
    char line[100];
    int j = 1; // line count

    file = fopen("input.txt", "r");
    if (file == NULL)
    {
        printf("can't open file.\n");
        return 1;
    }

    regex_t regex;
    regmatch_t match;

    const char *pattern[] = {
        "^(([[:space:]]*(int|char)[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*)(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*;[[:space:]]*)+$",
        "^[[:space:]]*((int|char)[[:space:]]+)?[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*((=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')+([[:space:]]*)*)?)*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?(,[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*)*;[[:space:]]*$",
    };

    size_t pattern_count = sizeof(pattern) / sizeof(pattern[0]);
    while (fgets(line, sizeof(line), file))
    {
        int found = 0;

        line[strcspn(line, "\n")] = '\0';

        for (int i = 0; i < pattern_count; i++)
        {
            int status = regcomp(&regex, pattern[i], REG_EXTENDED);

            if (status)
            {
                printf("Pattern in invalid\n");
                fclose(file);
                return -1;
            }
            int execute = regexec(&regex, line, 1, &match, 0);

            if (execute == 0)
            {
                found += 1;
            }
            regfree(&regex);
        }

        if (found)
        {
            printf("line %d: has transformation.\n", j);
        }
        else
        {
            printf("line %d: error.\n", j);
        }
        j++;
    }

    fclose(file);
    return 0;
}
