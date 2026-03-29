#include <stdio.h>
#include <stdbool.h>

typedef struct {
    float a;
    float b;
    float c;
} Cuboid;

/* 
   Ellenőrzött beállítás:
   csak pozitív élhosszakat fogad el.
   Siker esetén true, hiba esetén false.
*/
bool set_size(Cuboid* cuboid, float a, float b, float c)
{
    if (cuboid == NULL) {
        return false;
    }

    if (a <= 0 || b <= 0 || c <= 0) {
        return false;
    }

    cuboid->a = a;
    cuboid->b = b;
    cuboid->c = c;

    return true;
}

/* Térfogat */
float calc_volume(Cuboid cuboid)
{
    return cuboid.a * cuboid.b * cuboid.c;
}

/* Felszín */
float calc_surface(Cuboid cuboid)
{
    return 2 * (cuboid.a * cuboid.b +
                cuboid.a * cuboid.c +
                cuboid.b * cuboid.c);
}

/* Van-e négyzet alakú lapja? */
bool has_square_face(Cuboid cuboid)
{
    return (cuboid.a == cuboid.b ||
            cuboid.a == cuboid.c ||
            cuboid.b == cuboid.c);
}

int main()
{
    Cuboid c;

    if (!set_size(&c, 3, 4, 3)) {
        printf("Hibas meretek!\n");
        return 1;
    }

    printf("Elhosszak: %.2f %.2f %.2f\n", c.a, c.b, c.c);
    printf("Terfogat: %.2f\n", calc_volume(c));
    printf("Felszin: %.2f\n", calc_surface(c));

    if (has_square_face(c)) {
        printf("Van negyzet alaku lapja.\n");
    } else {
        printf("Nincs negyzet alaku lapja.\n");
    }

    return 0;
}