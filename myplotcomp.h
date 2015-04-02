#include <qwt_plot.h>

#define MAXDATA 5000

class QwtPlotCurve;
class QwtPlotMarker;

class MyPlotComp: public QwtPlot {
    Q_OBJECT
public:
    explicit MyPlotComp(QWidget *parent = 0);

    char dname[256];
    char fname[256];

    void err_Exit(char *location, char *reason, int exit_code);
    void loadData(char *dName);
    void usrDataFile(char *dname, char *fname);
    void previewPDD(char *dname, char *fname);

     // void showToolBar(QMainWindow *mWin);
	 void plotOF(char *mDirName, char *mFileName, char *cDirName, char *cFileName, float refDepth);

	 void changeSymbol();
	 void smoothing();
	 void showLegend();
	 void showDD();
     void showAllCurves();
	 void pddNorm();
	 void updatePlot();
	 
     void printPS(QString fName);

	 double xc[MAXDATA];   // x position
	 double dc[MAXDATA];	  // calculated dose
	 double ec[MAXDATA];	  // std in calculated dose
	 double dd[MAXDATA];	  // std in calculated dose	 
	 int    nc;            // number of calculated data points

	 double xm[MAXDATA];   // x position
	 double dm[MAXDATA];	  //  measured dose
	 double DM[MAXDATA];	  //  measured dose
	 int    nm;            // number of measured data points	 
	 double xz[MAXDATA];   // x position
	 double dz[MAXDATA];	  //  measured dose
	 int    nz;            // number of measured data points
	 double cFS[100]; // calculated Field Size
	 double cOF[100]; // calculated OF
	 int    ncFS;     // number of calculated FS
	 double mFS[100]; // calculated Field Size
	 double mOF[100]; // calculated OF
	 int    nmFS;     // number of calculated FS	 
	 
	 double  xPos;         // x Position of Profile
	 double  yPos;         // y Position of Profile
	 double  zPos;         // z Position of Profile
	 
	 int     nCurves;          // Number of Curves
	 QString     FilterSwitch;
	 QString     SymbolSwitch;
	 QString     LegendSwitch;
	 QString     DDSwitch;
	 QString     NormSwitch;
	 QString     MeasSwitch;
	 QString     CalcSwitch;
	 int     nFilterPoints;
	 QString Info;
	 
	 double xMin;
	 double xMax;
	 double yMin;
	 double yMax;
	 
	 float factor;
	 	 
private slots:



private:

     QString ddFile;
        
	 double dmMax[100];

     QwtPlotCurve *(crvc[100]);
     QwtPlotCurve *(crvm[100]);
     QwtPlotCurve *(crvd[100]);

     QwtPlotMarker *mrk1;
     QwtPlotMarker *mrk2;

     QwtPlotMarker *textMarker1;
     QwtPlotMarker *textMarker2;
     QwtPlotMarker *textMarker3;
  
};
