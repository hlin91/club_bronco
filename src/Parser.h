/* Header file for parsing functions for HTTP responses/requests */
#pragma once
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "Parser.c"
#define BUFF_MAX 1024

/*
    Parse an HTTP request and extract the request line, headers, and message
    Returns false on an empty string
    s: The request to parse
    req: String to store the extracted request line
    headers: Array of unallocated strings to store the extracted headers
    message: String to store the optional message
    numHeaders: Stores the number of headers parsed
*/
bool parseRequest(char *, char *, char **, char *, unsigned int *);

/*
    Parse a HTTP header into a key value pair
    Returns true when both a key and a value are successfully extracted, false otherwise
    s: The header to parse
    key: String to store the token to the left of the colon
    val: String to store the token to the right of the colon
*/
bool parseHeader(char *, char *, char *);

/*
    Parse an HTTP response and extract the status line, headers, and optional message
    This is just parseRequest with different semantics
    s: The response to parse
    status: String to store the status line
    headers: Array of unallocated strings to store the headers
    message: String to store the optional message
    numHeaders: Stores the number of headers parsed
*/
bool parseResponse(char *, char *, char **, char *, unsigned int *);