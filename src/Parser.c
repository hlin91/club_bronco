/* Parsing functions for HTTP requests/responses */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#define BUFF_MAX 1024

bool parseRequest(char *, char *, char **, char *, unsigned int *);
bool parseHeader(char *, char *, char *);
bool parseResponse(char *, char *, char **, char *, unsigned int *);

/*
    Parse an HTTP request and extract the request line, headers, and message
    Returns false on an empty string
    s: The request to parse
    req: String to store the extracted request line
    headers: Array of unallocated strings to store the extracted headers
    message: String to store the optional message
    numHeaders: Stores the number of headers parsed
*/
bool parseRequest(char *s, char *req, char **headers, char *message, unsigned int *numHeaders)
{
    char token[BUFF_MAX];
    char *p1 = s;
    char *p2 = NULL;
    /* Initialize return values */
    req[0] = '\0';
    headers[0] = NULL;
    message[0] = '\0';
    *numHeaders = 0;
    if (strlen(s) == 0)
        return false;
    /* Grab first line as request line */
    p2 = strchr(p1, '\n');
    if (p2 == NULL) /* Request is a one liner */
    {
        strcpy(req, s);
        return true;
    }
    strncpy(token, p1, p2 - p1);
    token[p2 - p1] = '\0';
    strcpy(req, token);
    /* Store subsequent lines as headers until NULL or blank line is encountered. Terminate with NULL */
    p1 = p2 + 1;
    p2 = strchr(p1, '\n');
    while (p2 != NULL)
    {
        strncpy(token, p1, p2 - p1);
        token[p2 - p1] = '\0';
        if (strlen(token) == 0) /* Encountered blank line */
            break;
        headers[*numHeaders] = (char*) malloc(strlen(token) * sizeof(char));
        strcpy(headers[*numHeaders], token);
        ++(*numHeaders);
        p1 = p2 + 1;
        p2 = strchr(p1, '\n');
    }
    headers[*numHeaders] = NULL;
    if (p2 == NULL)
        return true;
    /* Store the remainder as the message body */
    p1 = p2 + 1;
    strcpy(message, p1);
    return true;
}

/*
    Parse a HTTP header into a key value pair
    Returns true when both a key and a value are successfully extracted, false otherwise
    s: The header to parse
    key: String to store the token to the left of the colon
    val: String to store the token to the right of the colon
*/
bool parseHeader(char *s, char *key, char *val)
{
    unsigned int pos1 = 0;
    unsigned int pos2 = strlen(s);
    unsigned int i = pos1;
    /* Initialize return values */
    key[0] = '\0';
    val[0] = '\0';
    if (pos2 == 0) /* Empty string passed */
        return false;
    /* Split the header at the colon */
    while (s[i] != ':' && i <= pos2)
        ++i;
    if (i > pos2) /* No colon found */
        return false;
    strncpy(key, s, i - pos1);
    key[i - pos1] = '\0';
    ++i;
    if (i > pos2) /* No value found */
        return false;
    while (isspace(s[i]))
        ++i;
    strncpy(val, s + i, pos2 - i + 1);
    val[pos2 - i + 1] = '\0';
    return true;
}

/*
    Parse an HTTP response and extract the status line, headers, and optional message
    This is just parseRequest with different semantics
    s: The response to parse
    status: String to store the status line
    headers: Array of unallocated strings to store the headers
    message: String to store the optional message
    numHeaders: Stores the number of headers parsed
*/
bool parseResponse(char *s, char *status, char **headers, char *message, unsigned int *numHeaders)
{
    return parseRequest(s, status, headers, message, numHeaders);
}
