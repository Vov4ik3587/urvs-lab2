#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

double sinus(double x)
{
    int count = 1;
    double sum = 0;
    double f = x;
    double eps = 1e-15;

    while (fabs(f) > eps)
    {
        sum += f;
        f *= (-1.) * x * x / ((2. * count) * (2. * count + 1.));
        count++;
    }

    return sum;
}

double cosinus(double x)
{
    int count = 1;
    double sum = 0;
    double f = 1.;
    double eps = 1e-15;

    while (fabs(f) > eps)
    {
        sum += f;
        f *= (-1.) * x * x / ((2. * count) * (2. * count - 1.));
        count++;
    }

    return sum;
}

int main()
{
    struct storage
    {
        int num;     //идентификатор потомка
        double data; //значение, полученное от потомка
    } proc;

    int K, FunSinProc, FunCosProc, file_tmp; // file_tmp - файловый дескриптор .tmp
    double A, B, H;
    struct stat tmp_stat;

    file_tmp = open("file.tmp", O_CREAT | O_RDWR); // O_CREAT - создает файл, если еще его нет

    printf("Нижняя граница: ");
    if (!scanf("%lf", &A))
    {
        printf("\nНеверный тип нижнего предела\n");
        errno = EINVAL;
        exit(errno);
    } // einval - недопустимое значение для аргумента

    printf("Верхняя граница: ");
    if (!scanf("%lf", &B))
    {
        printf("\nНеверный тип верхнего предела\n");
        errno = EINVAL;
        exit(errno);
    } // einval - недопустимое значение для аргумента

    if (A > B)
    {
        printf("\nА должно быть не больше, чем В\n");
        errno = EINVAL;
        exit(errno);
    } // einval - недопустимое значение для аргумента

    printf("Количество разбиений: ");
    if (!scanf("%d", &K))
    {
        printf("\nНеверный тип количества разбиений\n");
        errno = EINVAL;
        exit(errno);
    } // einval - недопустимое значение для аргумента

    H = (B - A) / K; //длина отрезка разбиения

    FunSinProc = fork(); //создаем первый процесс-потомок
    if (FunSinProc == 0)
    {
        proc.num = getpid();
        proc.data = 0.0;
        double temp1 = sinus(A); //нижняя граница
        double temp2 = sinus(B); //верхняя граница
        double temp3 = 0.;       //между A и B

        double i = A + H;
        while (i < B)
        {
            temp3 += sinus(i);
            i += H;
        }

        double sum = temp1 + temp2 + 2 * temp3; //по формуле трапеций

        proc.data = sum;
        write(file_tmp, &proc, sizeof(proc)); //запись данных во временный файл
        exit(0);
    }

    FunCosProc = fork(); //создаем второй процесс-потомок
    if (FunCosProc == 0)
    {
        proc.num = getpid();
        proc.data = 0.0;
        double temp1 = cosinus(A); //нижняя граница
        double temp2 = cosinus(B); //верхняя граница
        double temp3 = 0.;         //между A и B

        double j = A + H;
        while (j < B)
        {
            temp3 += cosinus(j);
            j += H;
        }
        double sum = temp1 + temp2 + 2 * temp3; //формула трапеций
        proc.data = sum;
        write(file_tmp, &proc, sizeof(proc)); //запись данных во временный файл
        exit(0);
    }

    do
    {
        fstat(file_tmp, &tmp_stat);
    } while (tmp_stat.st_size != 2 * sizeof(proc));

    lseek(file_tmp, 0, SEEK_SET); //переходим в начало файла

    double sinA, cosA;

    read(file_tmp, &proc, sizeof(proc));
    if (proc.num == FunSinProc)
        sinA = proc.data;
    if (proc.num == FunCosProc)
        cosA = proc.data;
    //так как не знаем, какой процесс посчитается и запишется в файл раньше, читаем и проверяем два раза
    read(file_tmp, &proc, sizeof(proc));
    if (proc.num == FunSinProc)
        sinA = proc.data;
    if (proc.num == FunCosProc)
        cosA = proc.data;

    printf("%f %f\n", sinA, cosA);

    close(file_tmp);    //закрываем
    remove("file.tmp"); //удаляем

    waitpid(FunSinProc, NULL, 0);
    waitpid(FunCosProc, NULL, 0);

    double F = H / 2 * (sinA + cosA); //формула трапеций итоговая
    printf("Integral = %.10f\n", F);  //посчитанный интеграл по методу трапеций

    sleep(2);
    return 0;
}
