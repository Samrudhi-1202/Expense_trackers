
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CATEGORY_SIZE 50
#define DATE_SIZE 11
#define FILE_NAME "expenses.txt"

struct Expense {
    char date[DATE_SIZE];
    char category[CATEGORY_SIZE];
    float amount;
};

/* Remove remaining characters from the input line */
void clearInput(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

/* Read a line of text */
void readLine(char text[], int size) {
    if (fgets(text, size, stdin) != NULL) {
        size_t len = strlen(text);

        if (len > 0 && text[len - 1] == '\n') {
            text[len - 1] = '\0';
        } else {
            clearInput();
        }
    } else {
        text[0] = '\0';
    }
}

/* Check whether a date is valid: YYYY-MM-DD */
int validDate(const char date[]) {
    int year, month, day;
    int daysInMonth[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (strlen(date) != 10 ||
        date[4] != '-' || date[7] != '-') {
        return 0;
    }

    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 &&
            (date[i] < '0' || date[i] > '9')) {
            return 0;
        }
    }

    sscanf(date, "%4d-%2d-%2d", &year, &month, &day);

    if (year < 1 || month < 1 || month > 12) {
        return 0;
    }

    if (year % 400 == 0 ||
        (year % 4 == 0 && year % 100 != 0)) {
        daysInMonth[1] = 29;
    }

    return day >= 1 && day <= daysInMonth[month - 1];
}

/* Expand the dynamic array when needed */
int growArray(struct Expense **expenses,
              int *capacity, int count) {
    if (count < *capacity) {
        return 1;
    }

    int newCapacity = (*capacity == 0) ? 2 : (*capacity * 2);

    struct Expense *temp = realloc(
        *expenses, newCapacity * sizeof(struct Expense)
    );

    if (temp == NULL) {
        printf("Memory allocation failed!\n");
        return 0;
    }

    *expenses = temp;
    *capacity = newCapacity;
    return 1;
}

/* Save all expenses to the file */
int saveExpenses(struct Expense expenses[], int count) {
    FILE *file = fopen(FILE_NAME, "w");

    if (file == NULL) {
        printf("Error: Could not open %s for saving.\n", FILE_NAME);
        return 0;
    }

    int success = 1;

    for (int i = 0; i < count; i++) {
        if (fprintf(file, "%s|%s|%.2f\n",
                    expenses[i].date,
                    expenses[i].category,
                    expenses[i].amount) < 0) {
            success = 0;
            break;
        }
    }

    if (fclose(file) != 0) {
        success = 0;
    }

    if (!success) {
        printf("Error saving expense data.\n");
    }

    return success;
}

/* Load new-format and older category|amount records */
void loadExpenses(struct Expense **expenses,
                  int *count, int *capacity) {
    FILE *file = fopen(FILE_NAME, "r");
    char line[160];

    if (file == NULL) {
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        char *first = strchr(line, '|');

        if (first == NULL) {
            continue;
        }

        char date[DATE_SIZE] = "Unknown";
        char category[CATEGORY_SIZE];
        char *amountText;

        char *second = strchr(first + 1, '|');

        if (second != NULL) {
            /* New format: date|category|amount */
            *first = '\0';
            *second = '\0';

            if (strlen(line) != 10 || !validDate(line)) {
                continue;
            }

            snprintf(date, sizeof(date), "%s", line);
            snprintf(category, sizeof(category), "%s", first + 1);
            amountText = second + 1;
        } else {
            /* Old format: category|amount */
            *first = '\0';

            if (strlen(line) == 0 ||
                strlen(line) >= sizeof(category)) {
                continue;
            }

            snprintf(category, sizeof(category), "%s", line);
            amountText = first + 1;
        }

        char *end;
        float amount = strtof(amountText, &end);

        if (end == amountText || *end != '\0' ||
            amount < 0 || category[0] == '\0') {
            continue;
        }

        if (!growArray(expenses, capacity, *count)) {
            break;
        }

        snprintf((*expenses)[*count].date,
                 DATE_SIZE, "%s", date);
        snprintf((*expenses)[*count].category,
                 CATEGORY_SIZE, "%s", category);

        (*expenses)[*count].amount = amount;
        (*count)++;
    }

    fclose(file);
}

/* Display every expense */
void viewExpenses(struct Expense expenses[], int count) {
    if (count == 0) {
        printf("No expenses available.\n");
        return;
    }

    printf("\n----- ALL EXPENSES -----\n");

    for (int i = 0; i < count; i++) {
        printf("%d. Date: %s | Category: %s | Amount: %.2f\n",
               i + 1, expenses[i].date,
               expenses[i].category, expenses[i].amount);
    }
}

/* Calculate overall and category-wise totals */
void calculateTotals(struct Expense expenses[], int count) {
    float total = 0;

    for (int i = 0; i < count; i++) {
        total += expenses[i].amount;
    }

    printf("\nTotal of all expenses: %.2f\n", total);
    printf("\n----- CATEGORY-WISE TOTALS -----\n");

    for (int i = 0; i < count; i++) {
        int alreadyCounted = 0;

        for (int j = 0; j < i; j++) {
            if (strcmp(expenses[i].category,
                       expenses[j].category) == 0) {
                alreadyCounted = 1;
                break;
            }
        }

        if (alreadyCounted) {
            continue;
        }

        float categoryTotal = 0;

        for (int j = 0; j < count; j++) {
            if (strcmp(expenses[i].category,
                       expenses[j].category) == 0) {
                categoryTotal += expenses[j].amount;
            }
        }

        printf("%s: %.2f\n",
               expenses[i].category, categoryTotal);
    }
}

/* Search expenses by category */
void searchExpenses(struct Expense expenses[], int count) {
    char category[CATEGORY_SIZE];
    int found = 0;

    printf("Enter category to search: ");
    readLine(category, sizeof(category));

    for (int i = 0; i < count; i++) {
        if (strcmp(expenses[i].category, category) == 0) {
            printf("Date: %s | Category: %s | Amount: %.2f\n",
                   expenses[i].date,
                   expenses[i].category,
                   expenses[i].amount);
            found = 1;
        }
    }

    if (!found) {
        printf("No matching expense found.\n");
    }
}

/* Delete an expense by its displayed number */
void deleteExpense(struct Expense expenses[], int *count) {
    if (*count == 0) {
        printf("No expenses available to delete.\n");
        return;
    }

    viewExpenses(expenses, *count);

    printf("Enter expense number to delete: ");

    char input[30];
    readLine(input, sizeof(input));

    char *end;
    long number = strtol(input, &end, 10);

    if (end == input || *end != '\0' ||
        number < 1 || number > *count) {
        printf("Invalid expense number.\n");
        return;
    }

    int index = (int)number - 1;
    struct Expense deleted = expenses[index];

    for (int i = index; i < *count - 1; i++) {
        expenses[i] = expenses[i + 1];
    }

    (*count)--;

    if (saveExpenses(expenses, *count)) {
        printf("Expense deleted successfully!\n");
    } else {
        for (int i = *count; i > index; i--) {
            expenses[i] = expenses[i - 1];
        }

        expenses[index] = deleted;
        (*count)++;

        printf("Deletion cancelled because saving failed.\n");
    }
}

/* Report expenses for a selected month */
void monthlyReport(struct Expense expenses[], int count) {
    char month[8];
    float total = 0;
    int found = 0;

    printf("Enter month (YYYY-MM): ");
    readLine(month, sizeof(month));

    if (strlen(month) != 7 ||
        month[4] != '-' ||
        month[0] < '0' || month[0] > '9' ||
        month[1] < '0' || month[1] > '9' ||
        month[2] < '0' || month[2] > '9' ||
        month[3] < '0' || month[3] > '9' ||
        month[5] < '0' || month[5] > '9' ||
        month[6] < '0' || month[6] > '9' ||
        month[5] < '0' || month[5] > '1' ||
        (month[5] == '0' &&
         (month[6] < '1' || month[6] > '9')) ||
        (month[5] == '1' && month[6] > '2')) {
        printf("Invalid month. Use YYYY-MM, for example 2026-10.\n");
        return;
    }

    printf("\n----- MONTHLY REPORT: %s -----\n", month);

    for (int i = 0; i < count; i++) {
        if (strncmp(expenses[i].date, month, 7) == 0) {
            printf("%s | %s | %.2f\n",
                   expenses[i].date,
                   expenses[i].category,
                   expenses[i].amount);

            total += expenses[i].amount;
            found = 1;
        }
    }

    if (!found) {
        printf("No dated expenses found for this month.\n");
    }

    printf("Monthly total: %.2f\n", total);
}

int main(void) {
    struct Expense *expenses = NULL;
    int count = 0;
    int capacity = 0;
    int choice;

    loadExpenses(&expenses, &count, &capacity);

    printf("Loaded %d saved expense(s).\n", count);

    while (1) {
        printf("\n===== EXPENSE TRACKER =====\n");
        printf("1. Add Expense\n");
        printf("2. View Expenses\n");
        printf("3. Calculate Total\n");
        printf("4. Search Expense\n");
        printf("5. Delete Expense\n");
        printf("6. Monthly Report\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");

        char input[30];
        readLine(input, sizeof(input));

        char *end;
        long selected = strtol(input, &end, 10);

        if (end == input || *end != '\0' ||
            selected < 1 || selected > 7) {
            printf("Invalid choice. Select 1 to 7.\n");
            continue;
        }

        choice = (int)selected;

        switch (choice) {
            case 1: {
                if (!growArray(&expenses, &capacity, count)) {
                    break;
                }

                char date[DATE_SIZE];
                char category[CATEGORY_SIZE];

                printf("Enter date (YYYY-MM-DD): ");
                readLine(date, sizeof(date));

                if (!validDate(date)) {
                    printf("Invalid date. Example: 2026-10-10\n");
                    break;
                }

                printf("Enter expense category: ");
                readLine(category, sizeof(category));

                if (category[0] == '\0' ||
                    strchr(category, '|') != NULL) {
                    printf("Invalid category. Do not use |.\n");
                    break;
                }

                printf("Enter amount: ");
                readLine(input, sizeof(input));

                char *amountEnd;
                float amount = strtof(input, &amountEnd);

                if (amountEnd == input || *amountEnd != '\0' ||
                    amount < 0) {
                    printf("Invalid amount. Enter a non-negative number.\n");
                    break;
                }

                snprintf(expenses[count].date, DATE_SIZE, "%s", date);
                snprintf(expenses[count].category,
                         CATEGORY_SIZE, "%s", category);
                expenses[count].amount = amount;

                count++;

                if (saveExpenses(expenses, count)) {
                    printf("Expense added and saved successfully!\n");
                } else {
                    count--;
                    printf("Expense was not saved. Please try again.\n");
                }
                break;
            }

            case 2:
                viewExpenses(expenses, count);
                break;

            case 3:
                calculateTotals(expenses, count);
                break;

            case 4:
                if (count == 0) {
                    printf("No expenses available to search.\n");
                } else {
                    searchExpenses(expenses, count);
                }
                break;

            case 5:
                deleteExpense(expenses, &count);
                break;

            case 6:
                monthlyReport(expenses, count);
                break;

            case 7:
                free(expenses);
                printf("Thank you for using Expense Tracker!\n");
                return 0;
        }
    }
}
