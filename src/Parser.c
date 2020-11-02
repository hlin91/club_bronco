/* Parsing functions for HTTP requests/responses */

/*
  Parse a HTTP request and extract the request line, headers, and message
  s: The request to parse
  req: String to store the extracted request line
  headers: Array of strings to store the extracted headers
  message: String to store the optional message
*/
void parseRequest(char *s, char *req, char **headers, char *message)
{
    /* TODO: Implement this */
    /* Grab first line as request line */
    /* Store subsequent lines as headers until blank line is encountered. Terminate with NULL */
    /* Store the remainder as the message body */
}
