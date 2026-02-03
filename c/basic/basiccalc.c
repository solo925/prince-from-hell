# include <stdio.h>

int main(){
    int a, b;
    char op;
    printf("my basic calc\n");
    printf("Enter the first digit");
    scanf("%d", &a);
    printf("Enter the second digit");
    scanf("%d", &b);
    printf("Enter the operations (+, -, *, /): ");
    scanf(" %c", &op);

    switch (op)
    {
    case '+':
        int sum = a + b;
        printf("The sum is %d", sum);
        break;
    case '-':
        int diff = a - b;
        printf("The difference is %d", diff);
        break;
    case '*':
        int prod = a * b;
        printf("The product is %d", prod);
        break;
    case '/':
        if (b != 0) {
            float quot = (float)a / (float)b;
            printf("The quotient is %.2f", quot);
        } else {
            printf("Error: Division by zero is not allowed.");
        }
        break;
        
    default:
        break;
    }

    return 0;
}