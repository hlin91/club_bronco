/* Parsing functions for HTTP requests/responses */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define BUFF_MAX 1024

/*
  Parse an HTTP request and extract the request line, headers, and message
  s: The request to parse
  req: String to store the extracted request line
  headers: Array of unallocated strings to store the extracted headers
  message: String to store the optional message
  numHeaders: Stores the number of headers parsed
*/
void parseRequest(char *s, char *req, char **headers, char *message, unsigned int *numHeaders)
{
    /* TODO: Test this */
    char temp[BUFF_MAX] = "\0";
    strcpy(temp, s);
    char *token = NULL;
    /* Initialize return values */
    req[0] = '\0';
    headers[0] = NULL;
    message[0] = '\0';
    *numHeaders = 0;
    if (strlen(s) == 0)
        return;
    /* Tokenize copy of input string */
    token = strtok(temp, "\n");
    /* Grab first line as request line */
    strcpy(req, token);
    /* Store subsequent lines as headers until NULL or blank line is encountered. Terminate with NULL */
    token = strtok(NULL, "\n");
    while (token != NULL && strcmp(token, "\n") != 0)
    {
        headers[*numHeaders] = (char*) malloc(BUFF_MAX * sizeof(char));
        strcpy(headers[*numHeaders], token);
        ++(*numHeaders);
        token = strtok(NULL, "\n");
    }
    headers[*numHeaders] = NULL;
    /* Store the remainder as the message body */
    if (token != NULL)
        token = strtok(NULL, "\n");
    while (token != NULL)
    {
        strcat(message, token);
        token = strtok(NULL, "\n");
        if (token != NULL) /* If the token just added is not the last one */
            strcat(message, "\n"); /* Reintroduce the newline character */
    }
}

/*
  Parse a HTTP header into a key value pair
  s: The header to parse
  key: String to store the token to the left of the colon
  val: String to store the token to the right of the colon
*/
void parseHeader(char *s, char *key, char *val)
{
    /* TODO: Test this */
    unsigned int pos1 = 0;
    unsigned int pos2 = strlen(s);
    unsigned int i = pos1;
    /* Initialize return values */
    key[0] = '\0';
    val[0] = '\0';
    if (pos2 == 0) /* Empty string passed */
        return;
    /* Split the header at the colon */
    while (s[i] != ':' && i <= pos2)
        ++i;
    strncpy(key, s, i - pos1);
    key[i - pos1] = '\0';
    ++i;
    while (isspace(s[i]))
        ++i;
    strncpy(val, s + i, pos2 - i + 1);
    val[pos2 - i + 1] = '\0';
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
void parseResponse(char *s, char *status, char **headers, char *message, unsigned int *numHeaders)
{
    parseRequest(s, status, headers, message, numHeaders);
    return;
}
