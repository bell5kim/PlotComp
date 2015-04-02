#include "plotcomp.h"
#include <QApplication>

#include <cstdio>

void usage() {
  printf("\nUSAGE: %s -d directory\n", "plotComp.exe");
  printf("       directory calc or tmp\n");
  printf("\n");
}


int main(int argc, char *argv[])
{


    char *dName="";
    char *factor="1.000";

    if (argc < 2) {
       usage();
       return (0);
    }

    for(int iArg=0; iArg < argc; iArg++){
       if(iArg < argc-1){
          if( strcmp(argv[iArg],"-d") == 0 || strcmp(argv[iArg],"-dir") == 0) {
             iArg++;
             dName = argv[iArg];
          }
          if( strcmp(argv[iArg],"-f") == 0 || strcmp(argv[iArg],"-factor") == 0) {
             iArg++;
             factor = argv[iArg];
          }
       }
       if(strcmp("-help", argv[iArg]) == 0 || strcmp("-h", argv[iArg]) == 0 ) {
          usage();
          return(0);
       }
    }

    QApplication app(argc, argv);
    PlotComp w;
    w.setDataDir(dName);
    w.setFactor(factor);
    w.loadData(dName);
    w.show();

    return app.exec();
}
