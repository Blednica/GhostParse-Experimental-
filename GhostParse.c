#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_TOKEN 128

typedef struct NextToken
{
    char token[MAX_TOKEN];
    int count;
    struct NextToken *next;

} NextToken;

typedef struct State
{
    char token[MAX_TOKEN];
    int total;

    NextToken *nextTokens;

    struct State *next;

} State;

State *model = NULL;

void cleanToken(char *token)
{
    int len = strlen(token);

    while (len > 0 && isspace(token[len - 1]))
    {
        token[len - 1] = '\0';
        len--;
    }
}

State *findState(const char *token)
{
    State *current = model;

    while (current != NULL)
    {
        if (strcmp(current->token, token) == 0)
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

State *addState(const char *token)
{
    State *newState = malloc(sizeof(State));

    if (newState == NULL)
    {
        printf("Out of memory.\n");
        exit(1);
    }

    strcpy(newState->token, token);

    newState->total = 0;
    newState->nextTokens = NULL;

    newState->next = model;

    model = newState;

    return newState;
}

void addTransition(const char *currentToken, const char *nextToken)
{
    State *state = findState(currentToken);

    if (state == NULL)
    {
        state = addState(currentToken);
    }

    NextToken *next = state->nextTokens;

    while (next != NULL)
    {
        if (strcmp(next->token, nextToken) == 0)
        {
            next->count++;
            state->total++;

            return;
        }

        next = next->next;
    }

    NextToken *newNext = malloc(sizeof(NextToken));

    if (newNext == NULL)
    {
        printf("Out of memory.\n");
        exit(1);
    }

    strcpy(newNext->token, nextToken);

    newNext->count = 1;

    newNext->next = state->nextTokens;

    state->nextTokens = newNext;

    state->total++;
}

void trainFromFile(const char *fileName)
{
    FILE *f = fopen(fileName, "r");

    if (f == NULL)
    {
        printf("Could not open: %s\n", fileName);
        return;
    }

    char previous[MAX_TOKEN] = "";

    char current[MAX_TOKEN];

    while (fscanf(f, "%127s", current) == 1)
    {
        cleanToken(current);

        if (strlen(previous) > 0)
        {
            addTransition(previous, current);
        }

        strcpy(previous, current);
    }

    fclose(f);

    printf("Training complete.\n");
}

char *chooseNextToken(State *state)
{
    if (state == NULL || state->nextTokens == NULL)
    {
        return NULL;
    }

    int target = rand() % state->total;

    int running = 0;

    NextToken *current = state->nextTokens;

    while (current != NULL)
    {
        running += current->count;

        if (running > target)
        {
            return current->token;
        }

        current = current->next;
    }

    return NULL;
}

State *randomState()
{
    int count = 0;

    State *current = model;

    while (current != NULL)
    {
        count++;

        current = current->next;
    }

    if (count == 0)
    {
        return NULL;
    }

    int target = rand() % count;

    current = model;

    for (int i = 0; i < target; i++)
    {
        current = current->next;
    }

    return current;
}

void generateCode(int length)
{
    State *state = randomState();

    if (state == NULL)
    {
        printf("No model trained.\n");
        return;
    }

    char currentToken[MAX_TOKEN];

    strcpy(currentToken, state->token);

    printf("\n=========== GENERATED CODE ===========\n\n");

    for (int i = 0; i < length; i++)
    {
        printf("%s ", currentToken);

        if (strchr(currentToken, ';') ||
            strchr(currentToken, '{') ||
            strchr(currentToken, '}'))
        {
            printf("\n");
        }

        state = findState(currentToken);

        char *next = chooseNextToken(state);

        if (next == NULL)
        {
            state = randomState();

            if (state == NULL)
            {
                break;
            }

            strcpy(currentToken, state->token);
        }
        else
        {
            strcpy(currentToken, next);
        }
    }

    printf("\n\n======================================\n");
}

void printModelStats()
{
    int states = 0;
    int transitions = 0;

    State *current = model;

    while (current != NULL)
    {
        states++;

        NextToken *next = current->nextTokens;

        while (next != NULL)
        {
            transitions++;

            next = next->next;
        }

        current = current->next;
    }

    printf("\nModel Stats\n");
    printf("States: %d\n", states);
    printf("Transitions: %d\n\n", transitions);
}

void freeModel()
{
    State *state = model;

    while (state != NULL)
    {
        NextToken *next = state->nextTokens;

        while (next != NULL)
        {
            NextToken *tempNext = next;

            next = next->next;

            free(tempNext);
        }

        State *tempState = state;

        state = state->next;

        free(tempState);
    }

    model = NULL;
}

void menu()
{
    int choice;

    char filename[256];

    int length;

    while (1)
    {
        printf("\n=====================================\n");
        printf("       C Code Pattern AI Prototype\n");
        printf("=====================================\n");

        printf("1. Train from file\n");
        printf("2. Generate code\n");
        printf("3. Show model stats\n");
        printf("4. Clear model\n");
        printf("5. Exit\n");

        printf("Select option: ");

        scanf("%d", &choice);

        if (choice == 1)
        {
            printf("Enter filename: ");

            scanf("%255s", filename);

            trainFromFile(filename);
        }
        else if (choice == 2)
        {
            printf("How many tokens? ");

            scanf("%d", &length);

            generateCode(length);
        }
        else if (choice == 3)
        {
            printModelStats();
        }
        else if (choice == 4)
        {
            freeModel();

            printf("Model cleared.\n");
        }
        else if (choice == 5)
        {
            freeModel();

            printf("Exiting.\n");

            break;
        }
        else
        {
            printf("Invalid option.\n");
        }
    }
}

int main()
{
    srand((unsigned int)time(NULL));

    menu();

    return 0;
}
