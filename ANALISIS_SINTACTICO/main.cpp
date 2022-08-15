#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
using namespace std;

//#define TRUE 1
//#define FALSE 0

#define MAXTOKEN 50
#define NUMPALRES 4
#define MAX 50

//palabras reservadas dentro de la matriz
//puts sirve para escribir secuencias de caracteres
//char *PalRes[] = {"char","float","int","puts"}; //printf
char PalRes[5][10] = {"char","float","int","puts"}; //printf


char sLexema[127];
char sLinea[127];
//char *non[6] = "13579", *par[6] = "24680";
char non[6] = "13579", par[6] = "24680";
char asTokens [MAXTOKEN][100];
int indice = 0, edoAct, edoIni, iniToken = 0, k, numBytesArch;

int cima = -1;
char pilac[MAX][100];

void generarArch();
void vAbrirArchivo();
int bytesArch();

void vAnalisisLexico();
void vAnlisis_Sintactico();
void vIniEdos();
char nextChar();
void vRetract();
int edoAceptacion();
void vMuestra();
void falla();
void recuperarError();
int esId();

void insertarPila(string elem); //(char *elem); //o //(char elem[]);

int buscarTabla(char ap[], char x[]);

void eliminaPila();

int esToken(char x[]);

FILE *Fd;

char token[16][8] = {"x",";",",","*","id","[","]","num","char",
                     "int","float","puts","(",")","Cte.Lit","="};


char varsint[13][3] = {"x","D","L","L'","I","I'","A","A'","K","T","F","E","P"};

int tablaM[26][8] = {{1, 8, 1, 9, 2, -1, 999, 999},        //D->char
                     {1, 9, 1, 9, 2, -1, 999, 999},        //D->int
                     {1, 10, 1, 9, 2,-1, 999, 999},        //D->float
                     {2, 3, 2, 4, 3, 999, 999, 999},       //L->IL'    (*)
                     {2, 4, 2, 4, 3, 999, 999, 999},       //L->IL'    (id)
            /* 5 */  {3, 1, 3, 999, 999, 999, 999},        //L'->e     (;)
                     {3, 2, 3, -2, 4, 3, 999, 999},        //L'->,IL'  (,)
                     {4, 3, 4, -3, -4, 5, 999, 999},       //I->*idI   (*)
                     {4, 4, 4, -4, 5, 999, 999, 999},      //I->idI    (id)
                     {5, 1, 5, 999, 999, 999, 999},        //I'->e     (;)
           /* 10 */  {5, 2, 5, 999, 999, 999, 999},        //I'->e     (,)
                     {5, 5, 5, 6, 999, 999, 999, 999},     //I'->A     ([)
                     {5, 15, 5, -15,  8,   999, 999, 999}, //I'->=K    (=)
                     {6, 5, 6, -5, 8, -6, 7, 999},         //A->[k]A'  ([)
                     {7, 1, 7, 999, 999, 999, 999, 999},   //A'->e     (;)
                     {7, 2, 7, 999, 999, 999, 999, 999},   //A'->e     (,)
           /* 15 */  {7, 5, 6, -5, 8, -6, 7, 999},         //A'->[k]A' ([k])
                     {8, 4, 8, -4, 999, 999, 999, 999},    //K->id
                     {8, 7, 8, -7, 999, 999, 999, 999},    //K->num
                     {9, 8, 9, -8, 999, 999 ,999, 999},    //K->num
           /* -> */  {9, 9, 9, -9, 999, 999, 999, 999},    //T->int
           /* 20 */  {9, 10, 9, -10, 999, 999, 999, 999},  //T->float
        /*GRAMATICA DE PUTS*/
                     {10, 11, 10, -11, 11, -1, 999, 999},  //F->E
                     {11, 12, 11, -12, 12, -13, 999, 999}, //E->(P)
                     {12, 14, 12, -14, 999, 999, 999, 999},//P->Cte.lit
                     {12, 4, 12, -4, 999, 999, 999, 999}   //P->Id
                    };

int main(void)
{
    char resp;
    do{
        puts("DESEA GENERAR EL ARCHIVO(s/n) :");
        resp = cin.get();

        if(strchr("Ss",resp))
            generarArch();
        vAbrirArchivo();
        numBytesArch = bytesArch();
        if(numBytesArch == 0){
            printf("EL ARCHIVO NO TIENE DATOS. Pulse una tecla");
            if(cin.get() == 0){
                cin.get();
                exit(-1);
            }
        }
        else
            vAnalisisLexico();
        fclose(Fd);

        printf("SALIDA DEL ANALIZADOR LEXICO (asTokens)");
        vMuestra();
        printf("PULSE UNA TECLA PARA CONTINUAR");
        if(cin.get() == 0)
            cin.get();
        vAnlisis_Sintactico();
        cin.get();
        printf("PRESIONA (sS) PARA CONTINUAR ? : ");
        cin>>resp;
    }while(strchr("Ss",resp));
    return 0;
}

//ESTE METODO GENERA EL ARCHIVO DENTRO DEL PATH PROPORCIONADO
//UTILIZANDO LAS FUNCIONES DE fopen -que sirve para abrir y crear archivos en el disco
//(w+b) sirve para crear el archivo o abrirlo para actualizar(tanto para la entrada como para salida)
//(b) se incluye para abrir archivos binarios
//Y puts -es el encargado de escribir una cadena de flujos a la salida
void generarArch(){
    char car;
    char nomArch[100];

    string nombre;
    printf("NOMBRE DEL ARCHIVO A GENERAR(SIN EXTENSION): ");
    cin>>nombre;
    cin.get();

    sprintf(nomArch, "%s.txt", nombre.c_str());
    Fd = fopen(nomArch, "w+b");
    if(Fd == NULL){
        cout<<"NO SE PUEDE ABRIR EL ARCHIVO : "<<nomArch;
        exit(-1);
    }
    puts("TECLEA EL ARCHIVO : ");
    printf("TECLEA <|> PARA TERMINAR EL ARCHIVO \n");

    do{
        car = cin.get();
        if(car == '\n'){
            car = '\n';
            printf("\n");
        }
        if(car == 0) //o NULL
            car = cin.get(); //o getch();
        if(car != '|' && car != 8)
            fwrite(&car, sizeof (car), 1, Fd);
    }while(car != '|'); //27
    fclose(Fd);
}

//ESTE METODO ABRE EL ARCHIVO UTILIZANDO EL PATH PROPORCIONADO
//UTILIZNAOD LA FUNCION fopen -que sirve para abrir y crear archivos en el disco
//(r+b) sirve para leer y actualizar archivos binarios(tanto entrada como salida)el archivo debe de existir
void vAbrirArchivo(){
    char nomArch[100];
    char *nombre = new char[100];

    printf("\n\nNOMBRE DEL ARCHIVO A ABRIR(SIN EXTENSION) : ");
    cin.get();
    cin.getline(nombre, 100, '\n');

    sprintf(nomArch, "%s.txt", nombre);

    Fd = fopen(nomArch, "r+b");

    if(Fd == NULL){
        printf("NO SE PUEDE ABRIR EL ARCHIVO");
        exit(-1);
    }
    else
    {
        printf("EL ARCHIVO SE ENCUENTRA ABIERTO \n");
    }
}

//ESTE METODO PROPORCIONA LA CANTIDAD DE BYTES DEL ARCHIVO A ANALIZAR
//UTILIZA LAS FUNCIONES fseek PARA POSCICIONAR LOS PUNTEROS CON UNAS CONSTANTES
//SEEK?_END -SE VA AL FINAL DEL ARCHIVO,    SEEK_SET ESTABLECE EL PUNTERO A FILE EN LAS POSCICION INDICADA
//CON EL NUMERO DE BYTES
int bytesArch(){
    int aux;
    fseek(Fd, 0L, SEEK_END);
    aux = (int)ftell(Fd);
    fseek(Fd, 0L, SEEK_SET);
    return aux;
}

//ESTE METODO ANALIZA CADA CARACTER DENTRO DE DEL ARCHIVO
void vAnalisisLexico(){
    char cCarent;
    indice = iniToken = k = 0;
    vIniEdos();
    while(indice < numBytesArch || edoAceptacion()){
        switch(edoAct){
        case 0: cCarent = nextChar();
            if(strchr(non, cCarent))
                edoAct = 1;
            else
                if(strchr(par, cCarent))
                    edoAct = 2;
            else
                    falla();
            break;
        case 1: cCarent = nextChar();
            if(strchr(non, cCarent))
                    edoAct = 1;
            else
                if(strchr(par, cCarent))
                    edoAct = 2;
            else
                    falla();
            break;
        case 2: cCarent = nextChar();
            if(strchr(non, cCarent))
                edoAct = 1;
            else
                if(strchr(par, cCarent))
                    edoAct = 2;
            else
                    edoAct = 3;
            break;
        case 3: vRetract();
            strcpy(asTokens[k++],"num");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 4: cCarent = nextChar();
            switch(cCarent){
            case '+': edoAct = 5; break;
            case '-': edoAct = 6; break;
            case '*': edoAct = 7; break;
            case '/': edoAct = 8; break;
            case '=': edoAct=  31; break;
            default: falla();
            }
            break;
        case 5: strcpy(asTokens[k++], "+");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 6: strcpy(asTokens[k++], "-");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 7: strcpy(asTokens[k++], "*");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 8: strcpy(asTokens[k++], "/");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 9: cCarent = nextChar();
            if(strchr(par, cCarent))
                edoAct = 10;
            else
                if(strchr(non, cCarent))
                    edoAct = 11;
            else
                    falla();
            break;
        case 10: cCarent = nextChar();
            if(strchr(par, cCarent))
                edoAct = 10;
            else
                if(strchr(non, cCarent))
                    edoAct = 11;
            else
                    falla();
            break;
        case 11: cCarent = nextChar();
            if(strchr(non, cCarent))
                edoAct = 11;
            else
                if(strchr(par, cCarent))
                    edoAct = 10;
            else
                    edoAct = 12;
            break;
        case 12: vRetract();
            strcpy(asTokens[k++], "num");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 13: cCarent = nextChar();
            if((isalpha(cCarent) || cCarent == '_'))
                edoAct = 14;
            else
                falla();
            break;
        case 14: cCarent = nextChar();
            if((isalpha(cCarent) || cCarent =='_') || isdigit(cCarent))
                edoAct = 14;
            else
                edoAct = 15;
            break;
        case 15: vRetract();
            if(esId())
                strcpy(asTokens[k++], "id");
            else
                strcpy(asTokens[k++],sLexema);
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 16: cCarent = nextChar();
            if(cCarent == ';')
                edoAct = 17;
            else
                falla();
            break;
        case 17: strcpy(asTokens[k++], ";");
            strcpy(asTokens[k], "$");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 18: cCarent = nextChar();
            if(cCarent == '[')
                edoAct = 19;
            else
                falla();
            break;
        case 19: strcpy(asTokens[k++], "[");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 20: cCarent = nextChar();
            if(cCarent == ']')
                edoAct = 21;
            else
                falla();
            break;
        case 21: strcpy(asTokens[k++], "]");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 22: cCarent = nextChar();
            if(cCarent == ',')
                edoAct = 23;
            else
                falla();
            break;
        case 23: strcpy(asTokens[k++], ",");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 24: cCarent = nextChar();
            if(('"' == cCarent) && cCarent)
                edoAct = 25;
            else
                falla();
            break;
        case 25: cCarent = nextChar();
            if(('"' != cCarent) && cCarent)
                edoAct = 25;
            else
                if(cCarent)
                    edoAct = 26;
            break;
        case 26: strcpy(asTokens[k++], "Cte.Lit");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 27: cCarent = nextChar();
            if(cCarent == '(')
                edoAct = 28;
            else
                falla();
            break;
        case 28: strcpy(asTokens[k++], "(");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 29: cCarent = nextChar();
            if(cCarent == ')')
                edoAct = 30;
            else
                falla();
            break;
        case 30: strcpy(asTokens[k++], ")");
            if(indice >= numBytesArch)
                return;
            iniToken = indice;
            vIniEdos();
            break;
        case 31: strcpy(asTokens[k++], "=");
            if (indice >= numBytesArch)
                return;
            iniToken=indice;
            vIniEdos();
            break;
        }//FIN SWITCH
    }//FIN WHILE
}

//ESTE METODO SE ENCARGA DE ANALIZAR LAS PRODUCIONES
void vAnlisis_Sintactico(){
    int ip = 0, i, j;
    int renglon, iast;
    char x[10], a[10];
    insertarPila("$");
    if(strcmp(asTokens[ip],"puts")== 0)
        insertarPila("F");
    else
        insertarPila("D");

    printf("\nSALIDA DEL ANALIZADOR SINTACTICO (asTokens) \n\n");
    printf("ARREGLO DE TOKENS : \n\n");
    for(i = 0; strcmp(asTokens[i],"$") != 0; i++)
        printf("%s ", asTokens[i]);
    printf("\n\n PRODUCCIONES : \n\n");
    do
    {
        strcpy(x,pilac[cima]);
        strcpy(a,asTokens[ip]);
        if(esToken(x) || (strcmp(x,"$")== 0))
        {
            if(strcmp(x,a)==0)
            {
                eliminaPila();
                ip++;
            }else{
                if(strcmp(asTokens[ip],"puts")==0)
                    insertarPila("F");
                else
                    insertarPila("D");
                strcpy(x,pilac[cima]);

            }
        }else{
            renglon = buscarTabla(a,x);
            if(renglon != 999)
            {
                eliminaPila();
                iast = 0;
                printf("%-3s -> ",varsint[tablaM[renglon][0]]);
                for(j= 3; iast != 999; j++)
                {
                    iast = tablaM[renglon][j];  //999
                    if(iast < 0)
                    {
                        iast *= -1;
                        printf("%s",token[iast]);

                    }else{
                        if(iast !=999)
                            printf("%s",varsint[iast]);
                    }
                }
                printf("\n");
                for(i = j-2; i > 2; i--)
                {
                    iast = tablaM[renglon][i];
                    if(iast < 0)
                    {
                        iast *= -1;
                        insertarPila(token[iast]);
                    }else
                        insertarPila(varsint[iast]);
                }
            }else
            {
                printf("\n\n ERROR DE SINTAXIS ");
                return;
            }
        }
    }while(strcmp(x,"$")!= 0);
    printf("Cadena valida");
}

//ESTE METODO INICIALIZA A edoAct y edoIni a 0
void vIniEdos(){
    edoAct = 0;
    edoIni = 0;
}

//ESTE METODO RETORNA EL NUMERO DE CARACTERES LEIDOS DENTRO DEL Fd
//fread sirve para la lectura de caracteres
//&cAux indica la direcion en memoria
//sizeof(cAux)devolvera el mamaño en bytes dentro del archivo
char nextChar(){
    char cAux;
    fread(&cAux, sizeof(cAux), 1, Fd);
    indice++;
    return cAux;
}

//ESTE METODO DESPLAZA A LA POSICION DE INICIAL DEL ARCHIVO
void vRetract(){
    indice--;
    fseek(Fd, (long)indice, SEEK_SET);
}

//ESTE METODO TIENE LOS ESTADOS DE ACEPTACION DEL METODO de vAnalisisLexico
int edoAceptacion(){
    switch(edoAct){
    case 8:
    case 5:
    case 7:
    case 6:
    case 3:
    case 12:
    case 15:
    case 17:
    case 19:
    case 21:
    case 23:
    case 26:
    case 28:
    case 30:
    case 31:
        return true;
    default:
        return false;

    }
}

//ESTE METODO MUESTRA LOS TOKENS QUE ESTAN DENTRO DE asTokens[ij]
void vMuestra(){
    int iJ;
    puts("\nTOKENS RECONOCIDOS : \n");
    for(iJ = 0; iJ < k; iJ++){
        puts(asTokens[iJ]);
    }
}

//ESTE METODO SIRVE PARA CUANDO FALLA EL vAnalisisLexico
void falla(){
    switch(edoIni){
    case 0: edoIni = 4;
        indice = iniToken;
        fseek(Fd, (long)iniToken, SEEK_SET);
        break;
    case 4: edoIni = 9;
        indice = iniToken;
        fseek(Fd, (long)iniToken, SEEK_SET);
        break;
    case 9: edoIni = 13;
        indice = iniToken;
        fseek(Fd,(long)iniToken, SEEK_SET);
        break;
    case 13: edoIni= 16;
        indice = iniToken;
        fseek(Fd, (long)iniToken, SEEK_SET);
        break;
    case 16: edoIni = 18;
        indice = iniToken;
        fseek(Fd, (long)iniToken, SEEK_SET);
        break;
    case 18: edoIni = 20;
        indice = iniToken;
        fseek(Fd, (long)iniToken, SEEK_SET);
        break;
    case 20: edoIni = 22;
        indice = iniToken;
        fseek(Fd, (long)iniToken, SEEK_SET);
        break;
    case 22: edoIni = 24;
        indice = iniToken;
        fseek(Fd, (long)iniToken, SEEK_SET);
        break;
    case 24: edoIni = 27;
        indice = iniToken;
        fseek(Fd, (long)iniToken, SEEK_SET);
        break;
    case 27: edoIni = 29;
        indice = iniToken;
        fseek(Fd, (long)iniToken, SEEK_SET);
        break;
    case 29: recuperarError();
    }
    edoAct = edoIni;
}

//ESTE METODO SIRVE PARA RECUPERAR LOS ERROES QUE SE ENCUENTRARN EN iniToken
void recuperarError(){
    iniToken = indice;
    vIniEdos();
}

//ESTE METODO SIRVE PARA IDENTIFICAR LOS ID QUE ESTEN EN EL ARCHIVO
int esId(){
    int n, m, found = false;
    fseek(Fd, (long)iniToken, SEEK_SET);
    for(m = iniToken, n = 0; m < indice; m++, n++)
        fread(&sLexema[n], sizeof(char), 1, Fd);
    sLexema[n] = '\0';
    for(m = 0; m < NUMPALRES && !found; )
        if(strcmp(PalRes[m], sLexema) == 0)
            found = true;
        else
            m++;
    return (found ? 0 : 1);
}

//EN ESTE METODO SE INSERTAN LOS ELEMENTOS DENTRO DE LA PILA
//COPIANDO LOS ELEMENTOS A LA pilac Y PASANDO LA CADENA A cima
void insertarPila(string elem){
    if(cima == -1){
        cima = 0;
        strcpy(pilac[cima], elem.c_str());
    }else{
        if(cima == MAX -1)
            puts("PILA LLENA");
        else
        {
            cima++;
            strcpy(pilac[cima], elem.c_str());
        }
    }
}

//ESTE METODO BUSCA LOS TOKEN QUE ESTEN DENTRO DE LA TABLA M
//COMPARANDO CARACTER POR CARACTER
int buscarTabla(char a[], char x[]){
    int indx = 0, inda = 0, i;
    for(i = 0; i < 17; i ++)
        if(strcmp(a, token[i]) == 0)
            inda = i;
    for(i = 0; i < 14; i++)
        if(strcmp(x, varsint[i]) == 0)
            indx = i;
    for(i = 0; i < 27; i++){
        if(indx == tablaM[i][0])
            if(inda == tablaM[i][1])
                return i;
    }
    return 999;
}

//ESTE METODO SIRVE PARRA ELIMINAR LA PILA
//USNADO LA FUNCION strcpy QUE COPEA LA CADENA DENTRO DE OTRA CADENA
void eliminaPila(){
    if(cima == -1)
        puts("PILA VACIA");
    else
    {
        strcpy(pilac[cima],"");
        cima--;
    }
}

//ESTE METODO COMPARA CARACTER POR CARACTER DENTRO DEL ARREGLO
int esToken(char x[]){
    int i;
    for(i = 0; i < 16; i++){
        if(strcmp(x,token[i]) == 0)
            return 1;
    }
    return 0;
}
