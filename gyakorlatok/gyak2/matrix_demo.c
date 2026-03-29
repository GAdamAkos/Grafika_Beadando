#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define STACK_CAPACITY 100
#define PI 3.14159265358979323846

typedef struct {
    float data[3][3];
} Matrix;

typedef struct {
    float x;
    float y;
    float w;
} Point;

typedef struct {
    Matrix items[STACK_CAPACITY];
    int top;
} MatrixStack;


/*----------------------------------------------------------
  Alap műveletek
----------------------------------------------------------*/

void init_identity_matrix(Matrix* m)
{
    int i, j;

    if (m == NULL) {
        return;
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (i == j) {
                m->data[i][j] = 1.0f;
            } else {
                m->data[i][j] = 0.0f;
            }
        }
    }
}

void print_matrix(const Matrix* m)
{
    int i, j;

    if (m == NULL) {
        return;
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            printf("%8.3f ", m->data[i][j]);
        }
        printf("\n");
    }
}

void print_point(const Point* p)
{
    if (p == NULL) {
        return;
    }

    printf("(%.3f, %.3f, %.3f)\n", p->x, p->y, p->w);
}

void multiply_matrix_by_scalar(Matrix* m, float scalar)
{
    int i, j;

    if (m == NULL) {
        return;
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            m->data[i][j] *= scalar;
        }
    }
}

Matrix multiply_matrices(const Matrix* a, const Matrix* b)
{
    Matrix result;
    int i, j, k;

    init_identity_matrix(&result);

    if (a == NULL || b == NULL) {
        return result;
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            result.data[i][j] = 0.0f;
            for (k = 0; k < 3; k++) {
                result.data[i][j] += a->data[i][k] * b->data[k][j];
            }
        }
    }

    return result;
}

Point transform_point(const Matrix* m, const Point* p)
{
    Point result;

    result.x = 0.0f;
    result.y = 0.0f;
    result.w = 0.0f;

    if (m == NULL || p == NULL) {
        return result;
    }

    result.x = m->data[0][0] * p->x +
               m->data[0][1] * p->y +
               m->data[0][2] * p->w;

    result.y = m->data[1][0] * p->x +
               m->data[1][1] * p->y +
               m->data[1][2] * p->w;

    result.w = m->data[2][0] * p->x +
               m->data[2][1] * p->y +
               m->data[2][2] * p->w;

    return result;
}


/*----------------------------------------------------------
  Transzformációk
  A kapott mátrixot módosítják
----------------------------------------------------------*/

void scale(Matrix* m, float sx, float sy)
{
    Matrix s;

    if (m == NULL) {
        return;
    }

    init_identity_matrix(&s);
    s.data[0][0] = sx;
    s.data[1][1] = sy;

    *m = multiply_matrices(&s, m);
}

void shift(Matrix* m, float tx, float ty)
{
    Matrix t;

    if (m == NULL) {
        return;
    }

    init_identity_matrix(&t);
    t.data[0][2] = tx;
    t.data[1][2] = ty;

    *m = multiply_matrices(&t, m);
}

void rotate(Matrix* m, float angle_in_degrees)
{
    Matrix r;
    float rad;
    float c;
    float s;

    if (m == NULL) {
        return;
    }

    rad = angle_in_degrees * (float)PI / 180.0f;
    c = cosf(rad);
    s = sinf(rad);

    init_identity_matrix(&r);
    r.data[0][0] = c;
    r.data[0][1] = -s;
    r.data[1][0] = s;
    r.data[1][1] = c;

    *m = multiply_matrices(&r, m);
}


/*----------------------------------------------------------
  Verem műveletek
----------------------------------------------------------*/

void init_matrix_stack(MatrixStack* stack)
{
    if (stack == NULL) {
        return;
    }

    stack->top = -1;
}

bool push_matrix(MatrixStack* stack, Matrix m)
{
    if (stack == NULL) {
        return false;
    }

    if (stack->top >= STACK_CAPACITY - 1) {
        return false;
    }

    stack->top++;
    stack->items[stack->top] = m;
    return true;
}

bool pop_matrix(MatrixStack* stack, Matrix* m)
{
    if (stack == NULL || m == NULL) {
        return false;
    }

    if (stack->top < 0) {
        return false;
    }

    *m = stack->items[stack->top];
    stack->top--;

    return true;
}


/*----------------------------------------------------------
  Példák
----------------------------------------------------------*/

int main()
{
    Matrix m1;
    Matrix m2;
    Matrix result;
    MatrixStack stack;

    Point p = {2.0f, 1.0f, 1.0f};
    Point transformed;

    printf("=== 1. Egységmátrix ===\n");
    init_identity_matrix(&m1);
    print_matrix(&m1);

    printf("\n=== 2. Skalarral valo szorzas ===\n");
    multiply_matrix_by_scalar(&m1, 2.0f);
    print_matrix(&m1);

    printf("\n=== 3. Matrixszorzas ===\n");
    init_identity_matrix(&m1);
    init_identity_matrix(&m2);

    shift(&m1, 3.0f, 4.0f);
    scale(&m2, 2.0f, 2.0f);

    printf("m1 (eltolas):\n");
    print_matrix(&m1);

    printf("\nm2 (skalazas):\n");
    print_matrix(&m2);

    result = multiply_matrices(&m1, &m2);
    printf("\nm1 * m2:\n");
    print_matrix(&result);

    printf("\n=== 4. Pont transzformalasa ===\n");
    printf("Eredeti pont: ");
    print_point(&p);

    transformed = transform_point(&result, &p);
    printf("Transzformalt pont: ");
    print_point(&transformed);

    printf("\n=== 5. Kulon transzformacios fuggvenyek ===\n");
    init_identity_matrix(&m1);

    printf("Kezdo matrix:\n");
    print_matrix(&m1);

    shift(&m1, 5.0f, -2.0f);
    printf("\nEltolas utan:\n");
    print_matrix(&m1);

    scale(&m1, 2.0f, 3.0f);
    printf("\nSkalazas utan:\n");
    print_matrix(&m1);

    rotate(&m1, 90.0f);
    printf("\nForgatas utan:\n");
    print_matrix(&m1);

    transformed = transform_point(&m1, &p);
    printf("\nPont a teljes transzformacio utan: ");
    print_point(&transformed);

    printf("\n=== 6. Transzformacios verem ===\n");
    init_matrix_stack(&stack);

    init_identity_matrix(&m1);
    printf("Alap matrix:\n");
    print_matrix(&m1);

    push_matrix(&stack, m1);

    shift(&m1, 10.0f, 0.0f);
    printf("\nEltolas utan:\n");
    print_matrix(&m1);

    push_matrix(&stack, m1);

    rotate(&m1, 45.0f);
    printf("\nForgatas utan:\n");
    print_matrix(&m1);

    push_matrix(&stack, m1);

    scale(&m1, 0.5f, 0.5f);
    printf("\nSkalazas utan:\n");
    print_matrix(&m1);

    printf("\nVisszavonas pop_matrix segitsegevel:\n");

    if (pop_matrix(&stack, &m1)) {
        printf("1. visszaallitott matrix:\n");
        print_matrix(&m1);
    }

    if (pop_matrix(&stack, &m1)) {
        printf("\n2. visszaallitott matrix:\n");
        print_matrix(&m1);
    }

    if (pop_matrix(&stack, &m1)) {
        printf("\n3. visszaallitott matrix:\n");
        print_matrix(&m1);
    }

    return 0;
}