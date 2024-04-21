#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

Token *tokens; // single linked list of tokens
Token *lastTk; // the last token in list

int line = 1; // the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code)
{
	Token *tk = safeAlloc(sizeof(Token));
	tk->code = code;
	tk->line = line;
	tk->next = NULL;
	if (lastTk)
	{
		lastTk->next = tk;
	}
	else
	{
		tokens = tk;
	}
	lastTk = tk;
	return tk;
}

char *extract(const char *begin, const char *end)
{
	size_t length = end - begin;
	char *result = safeAlloc(length + 1);
	strncpy(result, begin, length);
	result[length] = '\0';
	return result;
}

Token *tokenize(const char *pch)
{
	const char *start;
	Token *tk;
	for (;;)
	{
		switch (*pch)
		{
		case ' ':
		case '\t':
			pch++;
			break;
		case '\r': // handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
			if (pch[1] == '\n')
				pch++;
			// fallthrough to \n
		case '\n':
			line++;
			pch++;
			break;
		case '\0':
			addTk(END);
			return tokens;
		case ',':
			addTk(COMMA);
			pch++;
			break;
		case '(':
			addTk(LPAR);
			pch++;
			break;
		case ')':
			addTk(RPAR);
			pch++;
			break;
		case '{':
			addTk(LACC);
			pch++;
			break;
		case '}':
			addTk(RACC);
			pch++;
			break;
		case '[':
			addTk(LBRACKET);
			pch++;
			break;
		case ']':
			addTk(RBRACKET);
			pch++;
			break;
		case ';':
			addTk(SEMICOLON);
			pch++;
			break;
		case '+':
			addTk(ADD);
			pch++;
			break;
		case '*':
			addTk(MUL);
			pch++;
			break;
		case '-':
			addTk(SUB);
			pch++;
			break;
		case '=':
			if (pch[1] == '=')
			{
				addTk(EQUAL);
				pch += 2;
			}
			else
			{
				addTk(ASSIGN);
				pch++;
			}
			break;
		case '&':
			if (pch[1] == '&')
			{
				addTk(AND);
				pch += 2;
			}
			else
			{
				err("simbol invalid: %c (%d)", *pch, *pch);
			}
			break;
		case '/':
			if (pch[1] == '/')
			{
				while (*pch != '\0' && *pch != '\n')
				{
					pch++;
				}
			}
			else
			{
				addTk(DIV);
				pch++;
			}
			break;
		case '.':
			if (isdigit(pch[1]) == 0)
			{
				addTk(DOT);
				pch++;
			}
			break;
		case '|':
			if (pch[1] == '|')
			{
				addTk(OR);
				pch += 2;
			}
			else
			{
				err("simbol invalid: %c (%d)", *pch, *pch);
			}
			break;
		case '!':
			if (pch[1] == '=')
			{
				addTk(NOTEQ);
				pch += 2;
			}
			else
			{
				addTk(NOT);
				pch++;
			}
			break;
		case '<':
			if (pch[1] == '=')
			{
				addTk(LESSEQ);
				pch += 2;
			}
			else
			{
				addTk(LESS);
				pch++;
			}
			break;
		case '>':
			if (pch[1] == '=')
			{
				addTk(GREATEREQ);
				pch += 2;
			}
			else
			{
				addTk(GREATER);
				pch++;
			}
			break;
		default:
			if (isalpha(*pch) || *pch == '_')
			{
				for (start = pch++; isalnum(*pch) || *pch == '_'; pch++)
				{
				}

				char *text = extract(start, pch);

				if (strcmp(text, "char") == 0)
					addTk(TYPE_CHAR);
				else if (strcmp(text, "int") == 0)
					addTk(TYPE_INT);
				else if (strcmp(text, "while") == 0)
					addTk(WHILE);
				else if (strcmp(text, "if") == 0)
					addTk(IF);
				else if (strcmp(text, "return") == 0)
					addTk(RETURN);
				else if (strcmp(text, "double") == 0)
					addTk(TYPE_DOUBLE);
				else if (strcmp(text, "else") == 0)
					addTk(ELSE);
				else if (strcmp(text, "struct") == 0)
					addTk(STRUCT);
				else if (strcmp(text, "void") == 0)
					addTk(VOID);
				else if (strcmp(text, "return") == 0)
					addTk(RETURN);
				else
				{
					tk = addTk(ID);
					tk->text = text;
				}
			}
			else if (isdigit(*pch))
			{
				for (start = pch++; isdigit(*pch) || *pch == '.' || *pch == 'E' || *pch == 'e' ||
									((*pch == 'e' || *pch == 'E') && (*(pch + 1) == '-' || (*(pch + 1) == '+' || isdigit(*(pch + 1))))) ||
									((*(pch - 1) == 'e' || *(pch - 1) == 'E') && (*pch == '-' || (*pch == '+' || isdigit(*pch))));
					 pch++)
				{
				}

				char *text = extract(start, pch);


				if (strchr(text, '.') || strchr(text, 'E') || strchr(text, 'e') || strchr(text, '-'))
				{
					if (strchr(text, '.'))
					{
						int i = 0;
						while (text[i] != '.')
						{
							i++;
						}
						if (!isdigit(text[i + 1]))
							err("invalid FORMAT of double at line: %d", tk->line);
					}
					if (strchr(text, 'E'))
					{
						int i = 0;
						while (text[i] != 'E')
						{
							i++;
						}
						if (!isdigit(text[i + 1])){
							if ((text[i + 1] == '+' || text[i + 1] == '-'))
							{
								if (!isdigit(text[i + 2]))
									err("invalid FORMAT of double at line: %d", tk->line);
							}
						}
						else
							err("invalid FORMAT of double at line: %d", tk->line);
					}
					if (strchr(text, 'e'))
					{
						int i = 0;
						while (text[i] != 'e')
						{
							i++;
						}
						if (!isdigit(text[i + 1])){
							if ((text[i + 1] == '+' || text[i + 1] == '-'))
							{
								if (!isdigit(text[i + 2]))
									err("invalid FORMAT of double at line: %d", tk->line);
							}
						}
						else
							err("invalid FORMAT of double at line: %d", tk->line);
					}
					double value = atof(text);
					tk = addTk(DOUBLE);
					tk->d = value;
				}
				else
				{
					int value = atoi(text);
					tk = addTk(INT);
					tk->i = (int)value;
				}
			}

			else if (isalpha(*pch) || *pch == '\'')
			{
				if ((*(pch + 1) == '\\') && (*(pch) == *(pch + 3)))
				{
					tk = addTk(CHAR);
					tk->c = *(pch + 2);
					pch = pch + 4;
				}
				else if (*(pch) == *(pch + 2))
				{
					tk = addTk(CHAR);
					tk->c = *(pch + 1);
					pch = pch + 3;
				}
				else
				{
					err("expected char at line: %d", tk->line);
				}
			}
			else if (isalpha(*pch) || *pch == '"')
			{

				if (*(pch + 1) == '"')
				{
					tk = addTk(STRING);
					tk->text = "";
					pch = pch + 2;
				}
				else
				{
					const char *start_string = pch;
					pch = pch + 1;
					while (*pch != '"')
					{
						pch++;
					}
					pch++;

					tk = addTk(STRING);
					tk->text = extract(start_string + 1, pch - 1);
				}
			}
		}
	}
}

void showTokens(const Token *tokens)
{
	int line_counter = 1;
	for (const Token *tk = tokens; tk; tk = tk->next)
	{
		printf("%d\t", tk->line);
		switch (tk->code)
		{
		case TYPE_INT:
			printf("TYPE_INT\n");
			break;
        case TYPE_DOUBLE:
            printf("TYPE_DOUBLE\n");
            break;
        case TYPE_CHAR:
            printf("TYPE_CHAR\n");
            break;
        case STRUCT:
            printf("STRUCT\n");
            break;
        case VOID:
            printf("VOID\n");
            break;
		case ID:
			printf("ID:%s\n", tk->text);
			break;
		case LPAR:
			printf("LPAR\n");
			break;
		case RPAR:
			printf("RPAR\n");
			break;
		case LACC:
			printf("LACC\n");
			break;
		case RACC:
			printf("RACC\n");
			break;
		case LBRACKET:
			printf("LBRACKET\n");
			break;
		case RBRACKET:
			printf("RBRACKET\n");
			break;
        case GREATER:
            printf("GREATER\n");
            break;
        case GREATEREQ:
            printf("GREATEREQ\n");
            break;
        case LESS:
            printf("LESS\n");
            break;
        case LESSEQ:
            printf("LESSEQ\n");
            break;
		case SEMICOLON:
			printf("SEMICOLON\n");
			break;
        case COMMA:
            printf("COMMA\n");
            break;
		case INT:
			printf("INT:%d\n", tk->i);
			break;
		case DOUBLE:
			printf("DOUBLE:%g\n", tk->d);
			break;
		case STRING:
			printf("STRING:%s\n", tk->text);
			break;
		case CHAR:
			printf("CHAR:%c\n", tk->c);
			break;
		case WHILE:
			printf("WHILE\n");
			break;
		case DIV:
			printf("DIV\n");
			break;
		case ADD:
			printf("ADD\n");
			break;
		case AND:
			printf("AND\n");
			break;
		case MUL:
			printf("MUL\n");
			break;
		case IF:
			printf("IF\n");
			break;
		case ELSE:
			printf("ELSE\n");
			break;
		case ASSIGN:
			printf("ASSIGN\n");
			break;
		case EQUAL:
			printf("EQUAL\n");
			break;
		case RETURN:
			printf("RETURN\n");
			break;
		case END:
			printf("END\n");
			break;
		case LINECOMMENT:
			printf("LINECOMMENT\n");
			break;
		case SPACE:
			printf("SPACE\n");
			break;
		case NOT:
			printf("NOT\n");
			break;
		case NOTEQ:
			printf("NOTEQ\n");
			break;
		default:
			printf("UNIDENTIFIED\n");
			break;
		}
		line_counter += 1;
	}
}
