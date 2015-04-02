#include <qwt_math.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_renderer.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_plot_canvas.h>
#include <qmath.h>
#include "myplotcomp.h"

#include <QDir>
#include <QPrinter>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

#include "medianFilter.h"
#include "pixmaps.h"

#ifndef XVMC
#define OF_SCALE ON
#endif
#define COMPFILE


MyPlotComp::MyPlotComp(QWidget *parent)
    :QwtPlot(parent)
{


    setAutoReplot( false );

    setTitle( "Verification of MC Calculation against Measurement" );

    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setBorderRadius( 10 );

    setCanvas( canvas );
    setCanvasBackground( QColor( "LightGray" ) );

    // legend
    // QwtLegend *legend = new QwtLegend;
    // insertLegend( legend, QwtPlot::BottomLegend );

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    grid->setMajorPen( Qt::white, 0, Qt::DotLine );
    grid->setMinorPen( Qt::gray, 0 , Qt::DotLine );
    grid->attach( this );

    // axes
    enableAxis( QwtPlot::yRight );
    setAxisTitle( QwtPlot::xBottom, "Distance from CAX (cm)" );
    setAxisTitle( QwtPlot::yLeft, "Output Factor (cGy/MU)" );
    // setAxisTitle( QwtPlot::yRight, "Phase [deg]" );

    // setAxisMaxMajor( QwtPlot::xBottom, 6 );
    // setAxisMaxMinor( QwtPlot::xBottom, 9 );
    // setAxisScaleEngine( QwtPlot::xBottom, new QwtLogScaleEngine );

    setAutoReplot( true );

}

void MyPlotComp::showLegend()
{
    if (LegendSwitch == "Show")
    {
        // legend
        QwtLegend *legend = new QwtLegend;
        legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
        // legend->setLegendFrameStyle(QFrame::NoFrame|QFrame::Plain);
        //insertLegend(legend, QwtPlot::BottomLegend);
        this->insertLegend(legend, QwtPlot::RightLegend);
    }
    else
    {
        this->insertLegend(NULL);
    }
    //QwtPlotCurve::updateLegend(legend);
    replot();
}

void MyPlotComp::showDD()
{
    if (DDSwitch == "Show")
    {
        // enableYRightAxis(TRUE);
        this->enableAxis(yRight,true);
        setAxisTitle(QwtPlot::yRight, "Percent Dose Difference");
    }
    else
    {
        this->enableAxis(yRight, false);
    }
    replot();
}


void MyPlotComp::showAllCurves()
{
    for (int i=0; i<=nCurves; i++)
    {

        //crvm[i] = new QwtPlotCurve(mLegend);
        //crvm[i]->setRenderHint(QwtPlotItem::RenderAntialiased);
        //crvm[i]->setPen(QPen(mPenColor));
        //crvm[i]->setYAxis(QwtPlot::yLeft);
        //crvm[i]->setSamples(xm, dm, nm);
        //crvm[i]->attach(this);

        crvm[i] = new QwtPlotCurve();
        crvc[i] = new QwtPlotCurve();
        QwtPlotCurve *cm = crvm[i];
        QwtPlotCurve *cc = crvc[i];
        if (MeasSwitch == "On")
        {
            //if ( cm ) cm->setEnabled(true);
        }
        else
        {
            //if ( cm ) cm->setEnabled(false);
        }
        if (CalcSwitch == "On")
        {
            //if ( cc ) cc->setEnabled(true);
        }
        else
        {
            //if ( cc ) cc->setEnabled(false);
        }
    }
    replot();
}



void MyPlotComp::printPS(QString fName)
{
#ifndef QT_NO_PRINTER
#if 0
    QPrinter printer;
#else
    QPrinter printer;
    //printer.setOutputToFile(true);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fName);
#endif

    QString docName = title().text();
    if ( docName.isEmpty() )
    {
        docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
        printer.setDocName (docName);
    }

    printer.setCreator("plotComp");
    printer.setOrientation(QPrinter::Landscape);
    printer.setPageSize(QPrinter::Letter);
    printer.setColorMode(QPrinter::Color);

    QwtPlotRenderer renderer;

    if ( printer.colorMode() == QPrinter::GrayScale )
    {
        renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
        renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
        renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
        renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
    }

    renderer.renderTo( this, printer );

#endif
}

// error handler
void MyPlotComp::err_Exit(char *location, char *reason, int exit_code)
{
    cerr << endl;
    cerr << "RUN-TIME ERROR in " << location << endl;
    cerr << "Reason: " << reason << "!" << endl;
    cerr << "Exiting to system..." << endl;
    cerr << endl;
    exit(exit_code);
}

/*
void MyPlotComp::toggleCurve(long curveId) {
    QwtPlotCurve *c = curve(curveId);
    if ( c ) {
        c->setEnabled(!c->enabled());
        replot();
    }
}
*/


void MyPlotComp::usrDataFile (char *dname, char *fname)
{

    QString PWD = getenv("PWD");
    QString mName = PWD.section('/',-2,-2);
    //strcpy(dName,dname);
    //strcpy(fName,fname);

    //QTextStream (stdout) << "usrDataFile:dname = " <<dname << endl;
    //QTextStream (stdout) << "usrDataFile:fname = " <<fname << endl;
    //QTextStream (stdout) << "usrDataFile: PWD = " << PWD << endl;
    //QTextStream (stdout) << "usrDataFile: mName = " << mName << endl;

    float sFactor = 10.0;
    float dFactor = factor; // Factor for Dose
#ifdef XVMC
    sFactor = 1.0;
#endif

    QDir *dirCalc = new QDir;
    dirCalc->setPath(dname);

    QDir *tmpDir = new QDir;
    tmpDir->setPath(".tmp");
    if (!tmpDir->exists())
    {
        tmpDir->setPath(".");
        tmpDir->mkdir(".tmp");
    }

    QString filefilter = QString(fname)+"*";
    dirCalc->setNameFilters(QStringList(filefilter));
    QStringList cStrList = dirCalc->entryList(QDir::Files, QDir::Name);
    cStrList.sort();
    //QTextStream (stdout) << cStrList.join('|') << endl;

//  float cpuTime=0.0;  char cpuTimeStr[256] = ""; bool cpuTimeFound=false;
//  float refDose=0.0;  float refDoseStd=0.0;  char refDoseStr[256] = ""; bool refDoseFound=false;

    QString logFile(fname);
    logFile=logFile.section(".p",0,0);
    QString dName(dname);
    dName = dName + "/";
    logFile = dName + logFile + ".log";
    //QTextStream (stdout) << "log File = " << logFile << endl;

    QString profile = "";
    if (filefilter.contains(".px")) profile = "x";
    if (filefilter.contains(".py")) profile = "y";
    if (filefilter.contains(".pz")) profile = "z";

    QString cpuTime="", calcDate="", HOST="unknown";
    QString MCVARIANCE="", ISO="";
    QString baseKey="";
#ifdef XVMC
    QString patientID="";
    QString refDose="", refDoseStd="";
#endif
    QString xVoxelSize="na", yVoxelSize="na", zVoxelSize="na";
    QString xSize="na", ySize="na", zSize="na";
    QString p_pri="", sigma_pri="", p_scat="", sigma_scat="";
    QString Emin="", Emax="";
    QString lvalue="", bvalue="", avalue="", zvalue="", nuvalue="";
    QString SSD="", sct_distance="";
    QString p_con="", distance_con="", radius_con="", e_mean_con="";
    QString X1="", X2="", Y1="", Y2="";
    QString H0="", H1="", H2="", H3="", H4="";
    QString norm_value="", gray_mu_dmax="";

    QFile file( logFile );
    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream( &file );
        QString line;
        while ( !stream.atEnd() )
        {
            line = stream.readLine(); // line of text excluding '\n'
            QString strLine = line.toLatin1();
            if (strLine.contains("Total CPU time:"))
            {
                cpuTime = strLine.section(':',1,1);
                cpuTime = cpuTime.section('s',0,0);
                bool ok;
                float cpu = cpuTime.simplified().toFloat(&ok);
                cpuTime.sprintf("%5.2f min",cpu/60);
            }
            if (strLine.contains("Date:"))
            {
                calcDate = strLine.section("Date:",1,1);
                calcDate = calcDate.simplified();
                QString Month = calcDate.section(' ',1,1);
                QString Day = calcDate.section(' ',2,2);
                QString Year = calcDate.section(' ',4,4);
                QString Time = calcDate.section(' ',3,3);
                calcDate = Month+" "+Day+", "+Year+" "+Time;
                calcDate = calcDate.simplified();
            }
            if (strLine.contains("!MCVARIANCE to"))
            {
                MCVARIANCE = strLine.section("to",1,1);
                MCVARIANCE = MCVARIANCE.remove(' ');
            }

            if (strLine.contains("Host:"))
            {
                HOST = strLine.section("Host:",1,1);
                HOST = HOST.section('.',0,0);
                HOST = HOST.remove(' ');
            }
#ifdef XVMC
            if (strLine.contains("Reference dose:"))
            {
                refDose = strLine.section(':',1,1);
                refDose = refDose.section("+/-",0,0);
                refDoseStd = strLine.section("+/-",1,1);
                refDoseStd = refDoseStd.section("Gy",0,0);
                bool ok;
                float rDoseSTD = refDoseStd.simplified().toFloat(&ok);
                float rDose    = refDose.simplified().toFloat(&ok);
                //QTextStream (stdout)  << "RSD = " << refDose << "/" << refDoseStd << "  " << rDose << "/" << rDoseSTD << endl;
                refDoseStd.sprintf("%4.1f",rDoseSTD/rDose*100);
            }
            if (strLine.contains("patient_id:"))
            {
                patientID = strLine.section("patient_id:",1,1);
                patientID = patientID.simplified();
            }
            if (strLine.contains("base_key:"))
            {
                baseKey = strLine.section("base_key:",1,1);
                baseKey = baseKey.simplified();
            }
#else
            if (strLine.contains("Initialize beam:") != 0)
            {
                baseKey = strLine.section("/",-1).section('.',0,0);
                baseKey = baseKey.simplified();
            }
#endif
            if (strLine.contains("p_pri:"))
            {
                p_pri = strLine.section("p_pri:",1,1);
                p_pri = p_pri.simplified();
                p_pri = p_pri.remove(' ');
            }
            if (strLine.contains("p_sct:"))
            {
                p_scat = strLine.section("p_sct:",1,1);
                p_scat = p_scat.simplified();
                p_scat = p_scat.remove(' ');
            }
            if (strLine.contains("sigma_pri:"))
            {
                sigma_pri = strLine.section("sigma_pri:",1,1);
                sigma_pri = sigma_pri.simplified();
                sigma_pri = sigma_pri.remove(' ');
            }
            if (strLine.contains("sigma_sct:"))
            {
                sigma_scat = strLine.section("sigma_sct:",1,1);
                sigma_scat = sigma_scat.simplified();
                sigma_scat = sigma_scat.remove(' ');
            }
            if (strLine.contains("sct_distance:"))
            {
                sct_distance = strLine.section("sct_distance:",1,1);
                sct_distance = sct_distance.simplified();
                sct_distance = sct_distance.remove(' ');
            }
            if (strLine.contains("energy_min:"))
            {
                Emin = strLine.section("energy_min:",1,1);
                Emin = Emin.simplified();
                Emin = Emin.remove(' ');
            }
            if (strLine.contains("energy_max:"))
            {
                Emax = strLine.section("energy_max:",1,1);
                Emax = Emax.simplified();
                Emax = Emax.remove(' ');
            }
            if (strLine.contains("l:"))
            {
                lvalue = strLine.section("l:",1,1);
                lvalue = lvalue.simplified();
                lvalue = lvalue.remove(' ');
            }
            if (strLine.contains("b:"))
            {
                bvalue = strLine.section("b:",1,1);
                bvalue = bvalue.simplified();
                bvalue = bvalue.remove(' ');
            }
            if (strLine.contains("norm_value:"))
            {
                norm_value = strLine.section("norm_value:",1,1);
                norm_value = norm_value.simplified();
                norm_value = norm_value.remove(' ');
            }
            if (strLine.contains("gray_mu_dmax:"))
            {
                gray_mu_dmax = strLine.section("gray_mu_dmax:",1,1);
                gray_mu_dmax = gray_mu_dmax.simplified();
                gray_mu_dmax = gray_mu_dmax.remove(' ');
            }
            if (strLine.contains("distance_con:"))
            {
                distance_con = strLine.section("distance_con:",1,1);
                distance_con = distance_con.simplified();
                distance_con = distance_con.remove(' ');
            }
            if (strLine.contains("radius_con:"))
            {
                radius_con = strLine.section("radius_con:",1,1);
                radius_con = radius_con.simplified();
                radius_con = radius_con.remove(' ');
            }
            if (strLine.contains("e_mean_con:"))
            {
                e_mean_con = strLine.section("e_mean_con:",1,1);
                e_mean_con = e_mean_con.simplified();
                e_mean_con = e_mean_con.remove(' ');
            }
#ifndef XVMC
            if (strLine.contains("XVMC> a:"))
            {
                avalue = strLine.section("a:",1,1);
                avalue = avalue.simplified();
                avalue = avalue.remove(' ');
            }
            if (strLine.contains("XVMC> z:"))
            {
                zvalue = strLine.section("z:",1,1);
                zvalue = zvalue.simplified();
                zvalue = zvalue.remove(' ');
            }
            if (strLine.contains("nu:"))
            {
                nuvalue = strLine.section("nu:",1,1);
                nuvalue = nuvalue.simplified();
                nuvalue = nuvalue.remove(' ');
            }
#endif
            if (strLine.contains("XVMC> horn_0:"))
            {
                H0 = strLine.section("horn_0:",1,1);
                H0 = H0.simplified();
                H0 = H0.remove(' ');
            }
            if (strLine.contains("XVMC> horn_1:"))
            {
                H1 = strLine.section("horn_1:",1,1);
                H1 = H1.simplified();
                H1 = H1.remove(' ');
            }
            if (strLine.contains("XVMC> horn_2:"))
            {
                H2 = strLine.section("horn_2:",1,1);
                H2 = H2.simplified();
                H2 = H2.remove(' ');
            }
            if (strLine.contains("XVMC> horn_3:"))
            {
                H3 = strLine.section("horn_3:",1,1);
                H3 = H3.simplified();
                H3 = H3.remove(' ');
            }
            if (strLine.contains("XVMC> horn_4:"))
            {
                H4 = strLine.section("horn_4:",1,1);
                H4 = H4.simplified();
                H4 = H4.remove(' ');
            }
            if (strLine.contains("p_con:"))
            {
                p_con = strLine.section("p_con:",1,1);
                p_con = p_con.simplified();
                p_con = p_con.remove(' ');
            }
            if (strLine.contains("FOUND DIMENSION"))
            {
                xSize = strLine.section("FOUND DIMENSION",1,1);
                xSize = xSize.simplified().section(" ",0,0);
                // xSize = xSize.remove(' ');
            }
            if (strLine.contains("FOUND DIMENSION"))
            {
                ySize = strLine.section("FOUND DIMENSION",1,1);
                ySize = ySize.simplified().section(" ",2,2);
                // ySize = ySize.remove(' ');
            }
            if (strLine.contains("FOUND DIMENSION"))
            {
                zSize = strLine.section("FOUND DIMENSION",1,1);
                zSize = zSize.simplified().section(" ",1,1);
                // zSize = zSize.remove(' ');
            }
            if (strLine.contains("FOUND GRIDSIZE"))
            {
                xVoxelSize = strLine.section("FOUND GRIDSIZE",1,1);
                xVoxelSize = xVoxelSize.simplified();
            }
            if (strLine.contains("FOUND GRIDSIZE"))
            {
                yVoxelSize = strLine.section("FOUND GRIDSIZE",1,1);
                yVoxelSize = yVoxelSize.simplified();
            }
            if (strLine.contains("FOUND GRIDSIZE"))
            {
                zVoxelSize = strLine.section("FOUND GRIDSIZE",1,1);
                zVoxelSize = zVoxelSize.simplified();
            }
#ifdef XVMC
            if (strLine.contains("iso_center.z:"))
            {
                SSD = strLine.section("iso_center.z:",1,1).section("cm",0,0);
                SSD = SSD.simplified();
                SSD = SSD.section(' ',1,1);
                SSD = SSD.simplified();
                bool ok;
                float isoDepth = 100 - SSD.toFloat(&ok);
                isoDepth = 100.0;
                SSD.sprintf("%6.0f cm",isoDepth);
            }
            if (strLine.contains("phantom x dim:"))
            {
                xSize = strLine.section("phantom x dim:",1,1);
                xSize = xSize.simplified();
            }
            if (strLine.contains("phantom y dim:"))
            {
                ySize = strLine.section("phantom y dim:",1,1);
                ySize = ySize.simplified();
            }
            if (strLine.contains("phantom z dim:"))
            {
                zSize = strLine.section("phantom z dim:",1,1);
                zSize = zSize.simplified();
            }
            if (strLine.contains("voxel x size:"))
            {
                xVoxelSize = strLine.section("voxel x size:",1,1);
                xVoxelSize = xVoxelSize.remove("cm");
                xVoxelSize = xVoxelSize.simplified();
            }
            if (strLine.contains("voxel y size:"))
            {
                yVoxelSize = strLine.section("voxel y size:",1,1);
                yVoxelSize = yVoxelSize.remove("cm");
                yVoxelSize = yVoxelSize.simplified();
            }
            if (strLine.contains("voxel z size:"))
            {
                zVoxelSize = strLine.section("voxel z size:",1,1);
                zVoxelSize = zVoxelSize.remove("cm");
                zVoxelSize = zVoxelSize.simplified();
            }
            //if (strLine.contains("iso_center.z:")) {
            //   SSD = strLine.section("iso_center.z:",1,1);
            //   SSD = SSD.remove("cm");
            //   SSD = SSD.simplified();
            //   bool ok;
            //   float isoDepth = 100 - SSD.toFloat(&ok);
            //   SSD.sprintf("%6.2fcm",isoDepth);
            //}
#else
            if (strLine.contains("FOUND ISO"))
            {
                SSD = strLine.section("FOUND ISO",1,1);
                ISO = SSD;
                SSD = SSD.simplified();
                SSD = SSD.section(' ',1,1);
                SSD = SSD.simplified();
                bool ok;
                float isoDepth = 1000 - SSD.toFloat(&ok);
                SSD.sprintf("%6.0f mm",isoDepth);
            }
            if (strLine.contains("transversal jaws:"))
            {
                X1 = strLine.section("transversal jaws:",1,1).section("to",0,0);
                X2 = strLine.section("transversal jaws:",1,1).section("to",1,1);
                X1 = X1.simplified();
                X2 = X2.simplified();
            }
            if (strLine.contains("parallel jaws:"))
            {
                Y1 = strLine.section("parallel jaws:",1,1).section("to",0,0);
                Y2 = strLine.section("parallel jaws:",1,1).section("to",1,1);
                Y1 = Y1.simplified();
                Y2 = Y2.simplified();
            }
#endif
        }
        file.close();
    }
//   cout << "Total CPU time: " << cpuTime << endl;


    double xmMin = 100*sFactor;
    double ymMin = 100*sFactor;
    double xmMax = -100*sFactor;
    double ymMax = -100*sFactor;

    //removeCurves();
    //removeMarkers();
    QwtPlotDict::detachItems(QwtPlotItem::Rtti_PlotCurve);
    QwtPlotDict::detachItems(QwtPlotItem::Rtti_PlotMarker);
    // QwtPlotDict::detachItems(QwtPlotItem::Rtti_PlotTextLabel);

    //QTextStream (stdout) << "usrDataFile: Total CPU time: " << cpuTime << endl;
    int i = 0;
    for ( QStringList::Iterator it = cStrList.begin(); it != cStrList.end(); ++it )
    {
        //QTextStream (stdout) << "usrDataFile: *it= " << *it << endl;
        QColor newColor = Qt::black;
        if (i == 0)  newColor = Qt::red;
        if (i == 1)  newColor = Qt::blue;
        if (i == 2)  newColor = Qt::green;
        if (i == 3)  newColor = Qt::magenta;
        if (i == 4)  newColor = Qt::cyan;
        if (i == 5)  newColor = Qt::darkYellow;
        if (i == 6)  newColor = Qt::darkRed;
        if (i == 7)  newColor = Qt::darkBlue;
        if (i == 8)  newColor = Qt::darkGreen;
        if (i == 9)  newColor = Qt::darkMagenta;
        if (i == 10) newColor = Qt::darkGray;
        if (i == 11) newColor = Qt::darkCyan;
        if (i == 12) newColor = Qt::gray;

        dmMax[i] = -1000;
        // Measured Curve  ----------------------
        QString mFNAME("meas/");
        mFNAME.append(*it);
        mFNAME.replace('p','m');

        //QTextStream (stdout) << "usrDataFile: mFNAME = " << mFNAME << endl;

        ifstream meas_file;
        meas_file.open(mFNAME.toLatin1(),ios::in);
        if (meas_file.bad()) err_Exit("PlotComp::usrDataFile","cannot open meas input file",8);
        int im = 0;

        while (!meas_file.eof())
        {
            char line[256] = "";  // lines to read from file
            meas_file.getline(line,sizeof(line));
            istringstream line_streamm(line);
            xm[im]=0.0;
            dm[im]=0.0; // initialization
            line_streamm >> xm[im] >> dm[im];
            // cout << im << "  " << xm[im] << "  " << dm[im] << endl;
            if (xm[im] <= xmMin) xmMin = xm[im];
            if (xm[im] >= xmMax) xmMax = xm[im];
            if (dm[im] <= ymMin) ymMin = dm[im];
            if (dm[im] >= ymMax) ymMax = dm[im];
            if (dm[im] >= dmMax[i]) dmMax[i] = dm[im];
            im++;
        }
        nm = im-1;
        // close file
        meas_file.close();

// Calculated Curve ---------------------
        //QTextStream (stdout) << "usrDataFile: *it = " << *it << endl;
        QString cFNAME(dName);
        cFNAME.append(*it);
#ifdef OF_SCALE
        QString zFNAME = cFNAME;
        if (cFNAME.contains(".px")) zFNAME = cFNAME.section(".px",0,0)+".pz0";
        if (cFNAME.contains(".py")) zFNAME = cFNAME.section(".py",0,0)+".pz0";
        if (cFNAME.contains(".px") || cFNAME.contains(".py"))
        {
            // cout << "zFNAME = " << zFNAME << endl;
            ifstream zfile;
            zfile.open(zFNAME.toLatin1(),ios::in);
            if (zfile.bad())
                err_Exit("PlotComp::usrDataFile","cannot open meas input file",8);
            int iz = 0;

            while (!zfile.eof())
            {
                char line[256] = "";  // lines to read from file
                zfile.getline(line,sizeof(line));
                istringstream line_streamz(line);
                xz[iz]=0.0;
                dz[iz]=0.0; // initialization
                line_streamz >> xz[iz] >> dz[iz];
                dz[im] *= dFactor;
                iz++;
            }
            nz = iz-2;
            zfile.close();
        }
#endif

        ifstream calc_file;
        calc_file.open(cFNAME.toLatin1(),ios::in);
        if (calc_file.bad())
            err_Exit("PlotComp::usrDataFile","cannot open calc input file",8);
        char comment[256] = ""; // Comment
        char xLabel[256] = "";  // string for x label
        char yLabel[256] = "";  // string for y label
        char zLabel[256] = "";  // string for z label

        char line0[256] = "";  // lines to read from file
        calc_file.getline(line0,sizeof(line0));
        istringstream line_stream0(line0);
        line_stream0 >> comment >> xLabel >> xPos;
        // cout << "Line0= " << comment << xLabel << xPos << endl;

        char line1[256] = "";  // lines to read from file
        calc_file.getline(line1,sizeof(line1));
        istringstream line_stream1(line1);
        line_stream1 >> comment >> yLabel >> yPos;
        // cout << "Line1= " << comment << yLabel << yPos << endl;

        char line2[256] = "";  // lines to read from file
        calc_file.getline(line2,sizeof(line2));
        istringstream line_stream2(line2);
        line_stream2 >> comment >> zLabel;
        // cout << "Line2= " << comment << zLabel << endl;

        zPos = -100*sFactor;
        if (strcmp(zLabel,"X") == 0)
        {
            zPos = yPos;
            yPos = xPos;
            xPos = -100*sFactor;
            setAxisTitle(QwtPlot::xBottom, "Distance from CAX (cm)");
        }
        if (strcmp(zLabel,"Y") == 0)
        {
            zPos = yPos;
            yPos = -100*sFactor;
            setAxisTitle(QwtPlot::xBottom, "Distance from CAX (cm)");
        }
        if (strcmp(zLabel,"Z") == 0)
        {
            setAxisTitle(QwtPlot::xBottom, "Depth (cm)");
        }

        QString titleText;
        QString FNAME = fname;
        QString ENG = FNAME.section(".",0,0);
        QString FS = FNAME.section(".",1,1);
        QString SSD = FNAME.section(".",2,2)+"0";
        QString PROF = FNAME.section(".",3,3).simplified();
        if (PROF.contains("px")) PROF = "X Profile";
        if (PROF.contains("py")) PROF = "Y Profile";
        if (PROF.contains("pz")) PROF = "Z Profile";
        QTextStream(&titleText) << PROF << " of "
                                << mName
                                << " for " << ENG << " "
                                << FS  << " SSD="
                                << SSD;
        setWindowTitle(titleText);


        int ic = 0;
        while (!calc_file.eof())
        {
            char line[256] = "";  // lines to read from file
            calc_file.getline(line,sizeof(line));
            istringstream line_streamc(line);
#ifdef XVMC
            line_streamc >> xc[ic] >> dc[ic] >> ec[ic];
            dc[ic] *= dFactor;
            ec[ic] *= dFactor;
#else
            line_streamc >> xc[ic] >> dc[ic];
            dc[ic] *= dFactor;
            ec[ic] *= dFactor;
            xc[ic] = xc[ic]/sFactor;
            if (cFNAME.contains("px")) xc[ic] = xc[ic] - yPos;  // 20.25 is the center of water phantom
            if (cFNAME.contains("py")) xc[ic] = xc[ic] - xPos;  // 20.25 is the center of water phantom
#endif
            for (int ii=0; ii<nm-1; ii++)
            {
                if ((xm[ii]-xc[ic])*(xm[ii+1]-xc[ic]) <= 0)
                {
                    dd[ic] = (dm[ii]*(xm[ii+1]-xc[ic])+dm[ii+1]*(xc[ic]-xm[ii]))
                             /(xm[ii+1]-xm[ii]);
                    dd[ic] = (dc[ic]-dd[ic])/dmMax[i]*100;
                    break;
                }
            }
            ic++;
        }
        nc = ic-2;
        // if (xc[nc] == 0.0 && dc[nc] == 0) nc--;
        // close file
        calc_file.close();

        if (FilterSwitch == "Filtered")
        {
            vector<double> adOriginal;
            for (int ic=0; ic<nc; ic++)
            {
                double dValue = dc[ic];
                adOriginal.push_back(dValue);
            }

            int iNumberOfFilterPoints = nFilterPoints;
            vector<double>* padFiltered = NULL;
            try
            {
                padFiltered = medianFilter<double>(adOriginal, iNumberOfFilterPoints);
                vector<double>& radFiltered = *padFiltered;
                int iNumberOfPoints = static_cast<int>(radFiltered.size());

                for (int iPoint = 0; iPoint < iNumberOfPoints; iPoint++)
                {
                    dc[iPoint] = radFiltered[iPoint];
                }

                delete padFiltered;
                padFiltered = NULL;
            }
            catch (const exception& roError)
            {
                delete padFiltered;
                padFiltered = NULL;

                cerr << roError.what() << endl;
                cerr << "UsrDataFile: Execution failed." << endl;
                exit(1);
            }
            catch (...)
            {
                delete padFiltered;
                padFiltered = NULL;

                cerr << "UsrDataFile: Unknown exception. Execution failed." << endl;
                exit(1);
            }

            delete padFiltered;
            padFiltered = NULL;
        }

#ifdef OF_SCALE
        if (cFNAME.contains(".px") || cFNAME.contains(".py"))
        {

            float cOF = 0.0;
            for (int ic=0; i<nc-1; ic++)
            {
                if (xc[ic]*xc[ic+1] < 0)
                {
                    cOF = (dc[ic]*xc[ic+1]+dc[ic+1]* -xc[ic])/(xc[ic+1]-xc[ic]);
                    break;
                }
                if (xc[ic] == 0)
                {
                    cOF = dc[ic];
                    break;
                }
            }

            float zOF = 0.0;
            if (zPos > 0)
            {
                for (int iz=0; iz<nz-1; iz++)
                {
                    if ((xz[iz]-zPos*sFactor)*(xz[iz+1]-zPos*sFactor) < 0)
                    {
                        zOF = (dz[iz]*(xz[iz+1]-zPos*sFactor)+dz[iz+1]*(zPos*sFactor-xz[iz]))
                              /(xz[iz+1]-xz[iz]);
                        break;
                    }
                    if (xz[iz] == zPos*sFactor)
                    {
                        zOF = dz[iz];
                        break;
                    }
                }
            }

            float oFactor = 1.0;
            if (cOF > 0.0) oFactor = zOF/cOF;
            // cout << "cOF = " << cOF << "  zOF = " << zOF << "  oFactor = " << oFactor << endl;
            for (int ic=0; ic<nc; ic++) dc[ic] = dc[ic] * oFactor *dFactor;

        }
#endif

        QString mLegend;
        QString mPenColor = "black";
        if (zPos > 0)
        {
            QTextStream(&mLegend) << "m: " << zPos*sFactor;
        }
        else
        {
            QTextStream(&mLegend) << "measured";
            // mPenColor = "blue";
        }
        //crvm[i] = insertCurve(mLegend);
        //setCurvePen(crvm[i], QPen(mPenColor));
        //setCurveYAxis(crvm[i], QwtPlot::yLeft);
        //setCurveData(crvm[i], xm, dm, nm);

        crvm[i] = new QwtPlotCurve(mLegend);
        crvm[i]->setRenderHint(QwtPlotItem::RenderAntialiased);
        crvm[i]->setPen(QPen(mPenColor));
        crvm[i]->setYAxis(QwtPlot::yLeft);
        crvm[i]->setSamples(xm, dm, nm);
        crvm[i]->attach(this);

        QString cLegend;
        if (zPos > 0)
        {
            QTextStream(&cLegend) << "c: " << zPos*sFactor;
        }
        else
        {
            QTextStream(&cLegend) << "calculated";
            newColor = Qt::red;
        }
        // crvc[i] = insertCurve(cLegend);
        //QwtSymbol sym;
        //sym.setStyle(QwtSymbol::Ellipse);
        //sym.setBrush(QBrush(newColor));
        //sym.setPen(QPen(newColor));
        //sym.setSize(5);
        // setCurveSymbol(crvc[i], sym);
        //setCurvePen(crvc[i], QPen(newColor,2));
        //setCurveYAxis(crvc[i], QwtPlot::yLeft);
        // setCurveStyle(crvc[i],QwtCurve::NoCurve,0);
        //setCurveData(crvc[i], xc, dc, nc);

        QwtSymbol *sym = new QwtSymbol();
        QPen pen(newColor, 2);
        //pen.setJoinStyle(Qt::MiterJoin);
        sym->setPen( pen );
        sym->setBrush(newColor);
        sym->setSize(5);

        crvc[i] = new QwtPlotCurve(cLegend);
        crvc[i]->setRenderHint(QwtPlotItem::RenderAntialiased);
        crvc[i]->setPen(QPen(newColor));
        crvc[i]->setYAxis(QwtPlot::yLeft);
        crvc[i]->setSamples(xc, dc, nc);
        crvc[i]->attach(this);


        if (DDSwitch == "Show")
        {
            QString dLegend;
            if (zPos > 0)
            {
                QTextStream(&dLegend) << "d: " << zPos*sFactor;
            }
            else
            {
                QTextStream(&dLegend) << "difference";
            }
            //crvd[i] = insertCurve(dLegend);
            //setCurvePen(crvd[i], QPen(Qt::darkYellow));
            //setCurveYAxis(crvd[i], QwtPlot::yRight);
            //setCurveData(crvd[i], xc, dd, nc);
            // toggleCurve(crvd[i]);

            crvd[i] = new QwtPlotCurve(dLegend);
            crvd[i]->setRenderHint(QwtPlotItem::RenderAntialiased);
            crvd[i]->setPen(QPen(Qt::darkYellow));
            crvd[i]->setYAxis(QwtPlot::yRight);
            crvd[i]->setSamples(xc, dd, nc);
            crvd[i]->attach(this);
        }
        // Create a comparison file in .tmp directory
        QString compFNAME(".tmp/");
        compFNAME.append(*it);
        compFNAME = compFNAME.section('.',0,3);
        QString depthText("");
        if (zPos > 0)
        {
            QTextStream(&depthText) << "." << zPos*sFactor << "."+profile+"Comp";
        }
        else
        {
            QTextStream(&depthText) << "."+profile+"Comp";
        }
        compFNAME.append(depthText);
        // cout << compFNAME << endl;
#ifdef COMPFILE
        ofstream out_file(compFNAME.toLatin1(),ios::out);
        if (out_file.bad())
        {
            err_Exit("usrDataFile to write","cannot open file",8);
        }
        for (int ic=0; ic<nc; ic++)
        {
            if (xc[ic] >= xmMin && xc[ic] <= xmMax)
            {
                // find corresponding value from dm
                for (int im=0; im<nm-1; im++)
                {
                    if ((xm[im]-xc[ic])*(xm[im+1]-xc[ic]) <= 0.0)
                    {
                        DM[ic] = (dm[im]*fabs(xm[im+1]-xc[ic])
                                  +dm[im+1]*fabs(xm[im]-xc[ic]))
                                 /fabs(xm[im+1]-xm[im]);
                        break;
                    }
                }
                out_file << xc[ic] << "   " << dc[ic] << "   " << DM[ic] << endl;
            }
        }
        out_file.close();
#endif
        i++;
    }
    nCurves = i - 1;   // Number of Curves

    QString FNAME = fname;
    // cout << FNAME << endl;
    QString FWFW = "";
    if (FNAME.contains(".px"))
    {
        FWFW = FNAME.section(".",1,1).section("x",0,0);
        QString FW = FWFW;
        // cout << "FWFW = " << FWFW << "  FW = " << FW << endl;
        bool ok;
        float fw = FW.toFloat(&ok)/10*1.5;
        // cout << "xmMin = " << xmMin << " xmMax = " << xmMax << " fw = " << fw << endl;
        if (xmMin < -fw) xmMin = -fw;
        if (xmMax > fw)  xmMax = fw;
    }

    if (FNAME.contains(".py"))
    {
        FWFW = FNAME.section(".",1,1).section("x",1,1);
        QString FW = FWFW;
        // cout << "FWFW = " << FWFW << "  FW = " << FW << endl;
        bool ok;
        float fw = FW.toFloat(&ok)/10*1.5;
        // cout << "xmMin = " << xmMin << " xmMax = " << xmMax << " fw = " << fw << endl;
        if (xmMin < -fw) xmMin = -fw;
        if (xmMax > fw)  xmMax = fw;
    }


    setAxisScale(QwtPlot::xBottom, xmMin, xmMax,0);
    // cout << "xmMin= " << xmMin << "  xmMax = " << xmMax << endl;
    setAxisScale(QwtPlot::yLeft, ymMin, ymMax*1.1,0);

    // textMarker1 = insertMarker();
    // setMarkerLineStyle(textPos, QwtMarker::VLine);
    //setMarkerPos(textMarker1, xmMin,ymMax*1.1+(ymMax-ymMin)*0.1/8*0.8);
    //setMarkerLabelAlign(textMarker1, Qt::AlignRight|Qt::AlignBottom);
    //setMarkerPen(textMarker1, QPen(Qt::blue, 0, Qt::DashDotLine));
    //setMarkerFont(textMarker1, QFont("Helvetica", 7, QFont::Normal));
    // setMarkerLabelText(textMarker1,"Ref Dose:" + refDose + "("+refDoseStd+"%)");
    textMarker1 = new QwtPlotMarker();
    textMarker1->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
    textMarker1->setLinePen(QPen(Qt::blue, 0, Qt::DashDotLine));
    //textMarker1->setLineStyle(QwtPlotMarker::VLine);
    textMarker1->setValue(xmMin,ymMax*1.1+(ymMax-ymMin)*0.1/8*0.8);
    //QTextStream (stdout) << "Text1 = " << (ymMax*1.1-ymMax)/5*1 << endl;
#ifdef OF_SCALE
    QString qsFactor = "1.000";
    qsFactor.sprintf("%6.4f",dFactor);
#endif
    QString Info1 = "Date:"+calcDate+" Phantom:"+xSize+"x"+ySize+"x"+zSize
                    +"("+xVoxelSize+"x"+yVoxelSize+"x"+zVoxelSize+")"
                    +" ISO:("+ISO
                    +") SSD:"+SSD
                    +" CPU:"+cpuTime+" on "+HOST
#ifndef XVMC

                    +" Jaws("+X1+":"+X2+","+Y1+":"+Y2+")"
                    +" MCVariance:"+MCVARIANCE
#endif
#ifdef OF_SCALE
                    +" Scaled by Factor of " + qsFactor
#endif
                    ; // Do not remove this line
    QwtText tMarker1(Info1);
    QFont tFont("Helvetica", 7, QFont::Normal);
    tMarker1.setFont(tFont);

    textMarker1->setLabel( tMarker1 );
    textMarker1->attach(this);

//    QwtText marker1(Info1);
//    QFont font1("Helvetica", 7, QFont::Normal);
//    marker1.setFont(font1);
//    QwtPlotTextLabel *markerItem1 = new QwtPlotTextLabel();
//    markerItem1->setText(marker1);
//    markerItem1->attach(this);
    //setMarkerLabelText(textMarker1,Info1);

    //textMarker2 = insertMarker();
    //setMarkerPos(textMarker2, xmMin,ymMax*1.1-(ymMax-ymMin)*0.1/8*1.0);
    //setMarkerLabelAlign(textMarker2, Qt::AlignRight|Qt::AlignBottom);
    //setMarkerPen(textMarker2, QPen(Qt::blue, 0, Qt::DashDotLine));
    //setMarkerFont(textMarker2, QFont("Helvetica", 7, QFont::Normal));

    textMarker2 = new QwtPlotMarker();
    textMarker2->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
    textMarker2->setLinePen(QPen(Qt::blue, 0, Qt::DashDotLine));
    // textMarker2->setLineStyle(QwtPlotMarker::VLine);
    //textMarker2->setValue(xmMin,ymMax*1.1-(ymMax-ymMin)*0.1/8*1.0);
    textMarker2->setValue(xmMin,ymMax*1.1-(ymMax-ymMin)*0.1/8*1.0);
    // QTextStream (stdout) << "Text2 = " << ymMax*1.1-(ymMax-ymMin)*0.1/8*1.0 << endl;

    QString Info2 = "BDT:"+baseKey+" Pp:"+p_pri+" Ps:"+p_scat+" Sp:"
                    +sigma_pri+" Ss:"+sigma_scat+" Pe:"+p_con
                    +" Emax:"+Emax+" Emin:"+Emin+" L:"+lvalue +" B:"+bvalue
                    +" A:"+avalue+" Z:"+zvalue+" Nu:"+nuvalue;

    QwtText tMarker2(Info2);
    tMarker2.setFont(tFont);

    textMarker2->setLabel( tMarker2 );
    textMarker2->attach(this);

//    QwtText marker2(Info1);
//    QFont font2("Helvetica", 7, QFont::Normal);
//    marker2.setFont(font2);
//    QwtPlotTextLabel *markerItem2 = new QwtPlotTextLabel();
//    markerItem2->setText(marker2);


    //textMarker3 = insertMarker();
    //setMarkerPos(textMarker3, xmMin,ymMax*1.1-(ymMax-ymMin)*0.1/8*3.0);
    //setMarkerLabelAlign(textMarker3, Qt::AlignRight|Qt::AlignBottom);
    //setMarkerPen(textMarker3, QPen(Qt::blue, 0, Qt::DashDotLine));
    //setMarkerFont(textMarker3, QFont("Helvetica", 7, QFont::Normal));

    textMarker3 = new QwtPlotMarker();
    textMarker3->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
    textMarker3->setLinePen(QPen(Qt::blue, 0, Qt::DashDotLine));
    // textMarker3->setLineStyle(QwtPlotMarker::VLine);
    textMarker3->setValue(xmMin,ymMax*1.1-(ymMax-ymMin)*0.1/8*3.0);
    //QTextStream (stdout) << "Text3 = " << ymMax*1.1-(ymMax-ymMin)*0.1/8*3.0 << endl;

    QString Info3 = "Zs:"+sct_distance+" Re:"+radius_con+" Ee:"+e_mean_con
                    + " h0:"+H0+" h1:"+H1+" h2:"+H2+" h3:"+H3+" h4:"+H4
                    + " N:"+norm_value+" Gy/MU at Dmax:"+gray_mu_dmax;

    QwtText tMarker3(Info3);
    tMarker3.setFont(tFont);

    textMarker3->setLabel( tMarker3 );
    textMarker3->attach(this);

//    QwtText marker3(Info3);
//    QFont font3("Helvetica", 7, QFont::Normal);
//    marker3.setFont(font3);
//    QwtPlotTextLabel *markerItem3 = new QwtPlotTextLabel();
//    markerItem3->setText(marker3);

    mName = PWD.section('/',-1);
    if (!mName.contains("PB")) textMarker2->attach(this);
    if (!mName.contains("PB")) textMarker3->attach(this);

    // replot();
    changeSymbol();
    Info = Info1 + " " + Info2 + " " + Info3;
}

void MyPlotComp::changeSymbol()
{
    // cout << "nCurves = " << nCurves << endl;
    // cout << "sw = " << sw << endl;
    QColor newColor = Qt::black;
    for (int i=0; i<=nCurves; i++)
    {
        if (i == 0)  newColor = Qt::red;
        if (i == 1)  newColor = Qt::blue;
        if (i == 2)  newColor = Qt::green;
        if (i == 3)  newColor = Qt::magenta;
        if (i == 4)  newColor = Qt::cyan;
        if (i == 5)  newColor = Qt::darkYellow;
        if (i == 6)  newColor = Qt::darkRed;
        if (i == 7)  newColor = Qt::darkBlue;
        if (i == 8)  newColor = Qt::darkGreen;
        if (i == 9)  newColor = Qt::darkMagenta;
        if (i == 10) newColor = Qt::darkGray;

        //QwtSymbol sym;
        //if (SymbolSwitch == "Dot") sym.setStyle(QwtSymbol::Ellipse);
        //if (SymbolSwitch == "Line") sym.setStyle(QwtSymbol::None);
        //sym.setBrush(QBrush(newColor));
        //sym.setPen(QPen(newColor));
        //sym.setSize(5);

        QwtSymbol *sym = new QwtSymbol();
        QPen pen(newColor, 2);
        if (SymbolSwitch == "Dot") sym->setStyle(QwtSymbol::Ellipse);
        if (SymbolSwitch == "Line") sym->setStyle(QwtSymbol::NoSymbol);
        sym->setPen( pen );
        sym->setBrush(newColor);
        sym->setSize(5);

        //setCurveSymbol(crvc[i], sym);
        //setCurvePen(crvc[i], QPen(newColor,2));
        //setCurveYAxis(crvc[i], QwtPlot::yLeft);
        //if (SymbolSwitch == "Dot") setCurveStyle(crvc[i],QwtCurve::NoCurve,0);
        //if (SymbolSwitch == "Line") setCurveStyle(crvc[i],QwtCurve::Lines,0);
        // setCurveData(crvc[i], xc, dc, nc);

        crvc[i]->setSymbol(sym);
        crvc[i]->setPen(QPen(newColor));
        crvc[i]->setYAxis(QwtPlot::yLeft);
        if (SymbolSwitch == "Dot")  crvc[i]->setStyle(QwtPlotCurve::NoCurve);
        if (SymbolSwitch == "Line") crvc[i]->setStyle(QwtPlotCurve::Lines);
        //crvc[i]->setSamples(xc, dd, nc);
        //crvc[i]->attach(this);
    }
    replot();
}

void MyPlotComp::smoothing()
{
    if (strlen(dname) > 1 && strlen(fname) > 1) usrDataFile(dname, fname);
}

void MyPlotComp::previewPDD (char *dname, char *fname)
{

    // cout << "dname = " << dname << "  fname = " << fname << endl;
    float sFactor = 10.0;
#ifdef XVMC
    sFactor = 1.0;
#endif

    QDir *dirCalc = new QDir;
    dirCalc->setPath(dname);

    QString filefilter = QString(fname)+"*";
    dirCalc->setNameFilters(QStringList(filefilter));

    QStringList cStrList = dirCalc->entryList(QDir::Files, QDir::Name);
    cStrList.sort();

    double xmMin = 100*sFactor;
    double ymMin = 100*sFactor;
    double xmMax = -100*sFactor;
    double ymMax = -100*sFactor;
    //removeCurves();
    //removeMarkers();
    QwtPlotDict::detachItems(QwtPlotItem::Rtti_PlotCurve);
    QwtPlotDict::detachItems(QwtPlotItem::Rtti_PlotMarker);

    int i = 0;
    for ( QStringList::Iterator it = cStrList.begin(); it != cStrList.end(); ++it )
    {
        QColor newColor = Qt::black;
        // QString newColor = "black";
        if (i == 0)  newColor = Qt::red;
        if (i == 1)  newColor = Qt::blue;
        if (i == 2)  newColor = Qt::green;
        if (i == 3)  newColor = Qt::magenta;
        if (i == 4)  newColor = Qt::cyan;
        if (i == 5)  newColor = Qt::darkYellow;
        if (i == 6)  newColor = Qt::darkRed;
        if (i == 7)  newColor = Qt::darkBlue;
        if (i == 8)  newColor = Qt::darkGreen;
        if (i == 9)  newColor = Qt::darkMagenta;
        if (i == 10) newColor = Qt::darkGray;
        if (i == 11) newColor = Qt::darkCyan;
        if (i == 12) newColor = Qt::gray;

        dmMax[i] = -1000;
        // Measured Curve  ----------------------
        //QString mFNAME("meas/");
        QString DNAME = dname;
        QString mFNAME(DNAME+"/");
        mFNAME.append(*it);
        // cout << i << "  " << newColor << "  "<< mFNAME << endl;
        ifstream meas_file;
        meas_file.open(mFNAME.toLatin1(),ios::in);
        if (meas_file.bad()) err_Exit("PlotComp::usrDataFile","cannot open meas input file",8);

        int im = 0;
        float localMax = 0.0;
        while (!meas_file.eof())
        {
            char line[256] = "";  // lines to read from file
            meas_file.getline(line,sizeof(line));
            istringstream line_streamm(line);
            xm[im]=0.0;
            dm[im]=0.0; // initialization
            line_streamm >> xm[im] >> dm[im];
            if (DNAME.contains("calc") || DNAME.contains("tmp")) xm[im]/=10.0;
            // cout << im << "  " << xm[im] << "  " << dm[im] << endl;
            if (xm[im] <= xmMin) xmMin = xm[im];
            if (xm[im] >= xmMax) xmMax = xm[im];
            if (dm[im] <= ymMin) ymMin = dm[im];
            if (dm[im] >= ymMax) ymMax = dm[im];
            if (dm[im] >= localMax) localMax = dm[im];
            if (dm[im] >= dmMax[i]) dmMax[i] = dm[im];
            im++;
        }
        nm = im-1;
        if (DNAME.contains("calc")||DNAME.contains("tmp")) nm--;

        if (NormSwitch.contains("Norm"))
        {
            for (int im=0; im<nm; im++)
                dm[im] = dm[im] / localMax * 100.0;
            ymMin = 0.0;
            ymMax = 105.0;
            setAxisTitle(QwtPlot::yLeft, "PPD to Dmax");
            setAxisTitle(QwtPlot::xBottom, "Depth (cm)");
        }
        else
        {
            setAxisTitle(QwtPlot::yLeft, "Output Factor (cGy/MU)");
            setAxisTitle(QwtPlot::xBottom, "Depth (cm)");
        }
        // close file
        meas_file.close();

        QString mLegend = mFNAME.section(".",1,1);
        //crvm[i] = insertCurve(mLegend);
        //setCurvePen(crvm[i], QPen(newColor,1));
        //setCurveYAxis(crvm[i], QwtPlot::yLeft);
        //setCurveData(crvm[i], xm, dm, nm);

        crvm[i] = new QwtPlotCurve(mLegend);
        crvm[i]->setRenderHint(QwtPlotItem::RenderAntialiased);
        crvm[i]->setPen(QPen(newColor));
        crvm[i]->setYAxis(QwtPlot::yLeft);
        crvm[i]->setSamples(xm, dm, nm);
        crvm[i]->attach(this);

        i++;
    }
    nCurves = i - 1;   // Number of Curves

    setAxisScale(QwtPlot::xBottom, xmMin, xmMax,0);
    // cout << "xmMin= " << xmMin << "  xmMax = " << xmMax << endl;
    setAxisScale(QwtPlot::yLeft, ymMin, ymMax*1.1,0);
    // cout << "ymMin= " << ymMin << "  ymMax = " << ymMax << endl;

    replot();
    // changeSymbol();

}

void MyPlotComp::pddNorm()
{
    previewPDD("meas", "*.mz0");
}

void MyPlotComp::plotOF(char *mDirName, char *mFileName, char *cDirName, char *cFileName, float refDepth)
{

    float dFactor = factor;

    //removeCurves();
    //removeMarkers();
    QwtPlotDict::detachItems(QwtPlotItem::Rtti_PlotCurve);
    QwtPlotDict::detachItems(QwtPlotItem::Rtti_PlotMarker);

    refDepth /= 10.0;

    QDir *dirMeas = new QDir;
    dirMeas->setPath(mDirName);

    //QStringList mfilefilter; // file name for measured data
    //mfilefilter << mFileName << "*"; // file name for measured data
    //dirMeas->setNameFilters(mfilefilter);

    QString mfilefilter = QString(mFileName)+"*";
    dirMeas->setNameFilters(QStringList(mfilefilter));

    QStringList mStrList = dirMeas->entryList(QDir::Files, QDir::Name);
    mStrList.sort();

    QDir *dirCalc = new QDir;
    dirCalc->setPath(cDirName);

    //QStringList cfilefilter; // file name for measured data
    //cfilefilter << cFileName << "*"; // file name for measured data
    //dirCalc->setNameFilters(cfilefilter);

    QString cfilefilter = QString(cFileName)+"*";
    dirCalc->setNameFilters(QStringList(cfilefilter));

    QStringList cStrList = dirCalc->entryList(QDir::Files, QDir::Name);
    cStrList.sort();

    int nmFS = 0;
    for ( QStringList::Iterator it = mStrList.begin(); it != mStrList.end(); ++it )
    {

        // Measured OF  ----------------------
        //QString mFNAME("meas/");
        QString DNAME = mDirName;
        QString mFNAME(DNAME+"/");
        mFNAME.append(*it);
        ifstream meas_file;
        meas_file.open(mFNAME.toLatin1(),ios::in);
        if (meas_file.bad()) err_Exit("PlotComp::plotOF","cannot open a measured input file",8);

        int im = 0;
        while (!meas_file.eof())
        {
            char line[256] = "";  // lines to read from file
            meas_file.getline(line,sizeof(line));
            istringstream mline_stream(line);
            xm[im]=0.0;
            dm[im]=0.0; // initialization
            mline_stream >> xm[im] >> dm[im];
            im++;
        }
        nm = im-1;
        if (DNAME.contains("meas")) nm--;

        for (int im=1; im<nm; im++)
        {
            // cout << xm[im] << "  " << refDepth << endl;
            if ((xm[im]-refDepth)*(xm[im-1]-refDepth)<=0.0)
            {
                QString FS = mFNAME.section("x",1,1).section(".",0,0);
                bool ok;
                mFS[nmFS] = FS.toFloat(&ok);
                mOF[nmFS] = dm[im-1]+(dm[im]-dm[im-1])/(xm[im]-xm[im-1])*(refDepth-xm[im-1]);
                nmFS++;
                break;
            }
        }

        // close file
        meas_file.close();
    }
    // Sorting
    xMin = 0;
    xMax = 0;
    yMin = mOF[0];
    yMax = 0;
    for (int i=0; i<nmFS; i++)
    {
        for (int j=i; j<nmFS; j++)
        {
            if (mFS[j]<=mFS[i])
            {
                float tmpFS = mFS[i];
                float tmpOF = mOF[i];
                mFS[i]=mFS[j];
                mOF[i]=mOF[j];
                mFS[j]=tmpFS;
                mOF[j]=tmpOF;
            }
        }
        //cout << "meas " << i << " " << mFS[i] << "  " << mOF[i] << endl;

        if (mFS[i] > xMax) xMax = mFS[i];
        if (mOF[i] < yMin) yMin = mOF[i];
        if (mOF[i] > yMax) yMax = mOF[i];
    }


    //QwtSymbol mSym;
    //mSym.setStyle(QwtSymbol::Ellipse);
    //mSym.setBrush(QBrush("blue"));
    //mSym.setPen(QPen("blue"));
    //mSym.setSize(5);

    QwtSymbol *mSym = new QwtSymbol();
    QPen mPen(Qt::blue, 2);
    //pen.setJoinStyle(Qt::MiterJoin);
    mSym->setPen( mPen );
    mSym->setBrush(QBrush(Qt::blue));
    mSym->setSize(5);


    QString mLegend = "measured";
    //crvm[0] = insertCurve(mLegend);
    //setCurveSymbol(crvm[0], mSym);
    //setCurvePen(crvm[0], QPen("blue",1));
    //setCurveYAxis(crvm[0], QwtPlot::yLeft);
    //setCurveData(crvm[0], mFS, mOF, nmFS);

    crvm[0] = new QwtPlotCurve(mLegend);
    crvm[0]->setRenderHint(QwtPlotItem::RenderAntialiased);
    crvm[0]->setPen(Qt::blue);
    crvm[0]->setSymbol(mSym);
    crvm[0]->setYAxis(QwtPlot::yLeft);
    crvm[0]->setSamples(mFS, mOF, nmFS);
    crvm[0]->attach(this);

    int ncFS = 0;
    for ( QStringList::Iterator it = cStrList.begin(); it != cStrList.end(); ++it )
    {

        // Calculated OF  ----------------------
        //QString mFNAME("meas/");
        QString DNAME = cDirName;
        QString cFNAME(DNAME+"/");
        cFNAME.append(*it);
        ifstream calc_file;
        calc_file.open(cFNAME.toLatin1(),ios::in);
        if (calc_file.bad()) err_Exit("PlotComp::plotOF","cannot open a calculated input file",8);

        char comment[256] = ""; // Comment
        char xLabel[256] = "";  // string for x label
        char yLabel[256] = "";  // string for y label
        char zLabel[256] = "";  // string for z label

        char line0[256] = "";  // lines to read from file
        calc_file.getline(line0,sizeof(line0));
        istringstream line_stream0(line0);
        line_stream0 >> comment >> xLabel >> xPos;
        // cout << "Line0= " << comment << xLabel << xPos << endl;

        char line1[256] = "";  // lines to read from file
        calc_file.getline(line1,sizeof(line1));
        istringstream line_stream1(line1);
        line_stream1 >> comment >> yLabel >> yPos;
        // cout << "Line1= " << comment << yLabel << yPos << endl;

        char line2[256] = "";  // lines to read from file
        calc_file.getline(line2,sizeof(line2));
        istringstream line_stream2(line2);
        line_stream2 >> comment >> zLabel;
        // cout << "Line2= " << comment << zLabel << endl;

        int ic = 0;
        while (!calc_file.eof())
        {
            char line[256] = "";  // lines to read from file
            calc_file.getline(line,sizeof(line));
            istringstream cline_stream(line);
            xc[ic]=0.0;
            dc[ic]=0.0; // initialization
#ifdef XVMC
            cline_stream >> xc[ic] >> dc[ic] >> ec[ic];
            xc[ic] *= 10.0;
            // cout << xc[ic] << " " << dc[ic] << " " << ec[ic] << endl;
#else
            cline_stream >> xc[ic] >> dc[ic];
#endif
            if (DNAME.contains("calc")||DNAME.contains("tmp")) xc[ic]/=10.0;
            ic++;
        }
        nc = ic-1;
        if (DNAME.contains("calc")||DNAME.contains("tmp")) nc--;
        for (int ic=1; ic<nc; ic++)
        {
            // cout << xc[ic] << " " << dc[ic] << " " << ec[ic] << endl;
            if ((xc[ic]-refDepth)*(xc[ic-1]-refDepth)<=0.0)
            {
                QString FS = cFNAME.section("x",1,1).section(".",0,0);
                bool ok;
                cFS[ncFS] = FS.toFloat(&ok);
                cOF[ncFS] = dc[ic-1]+(dc[ic]-dc[ic-1])/(xc[ic]-xc[ic-1])*(refDepth-xc[ic-1]);
                cOF[ncFS] *= dFactor;
// cout << "calc " << ncFS << " " << cFS[ncFS] << "  " << cOF[ncFS] << endl;
                ncFS++;
                break;
            }
        }

        // close file
        calc_file.close();

    }
    // Sorting
    for (int i=0; i<ncFS; i++)
    {
        for (int j=i; j<ncFS; j++)
        {
            if (cFS[j]<=cFS[i])
            {
                float tmpFS = cFS[i];
                float tmpOF = cOF[i];
                cFS[i]=cFS[j];
                cOF[i]=cOF[j];
                cFS[j]=tmpFS;
                cOF[j]=tmpOF;
            }
        }
        // cout << "meas " << i << " " << mFS[i] << "  " << mOF[i] << endl;
    }
    //QwtSymbol cSym;
    //cSym.setStyle(QwtSymbol::Ellipse);
    //cSym.setBrush(QBrush("red"));
    //cSym.setPen(QPen("red"));
    //cSym.setSize(5);

    QwtSymbol *cSym = new QwtSymbol();
    QPen pen(Qt::red, 2);
    //pen.setJoinStyle(Qt::MiterJoin);
    cSym->setPen( pen );
    cSym->setBrush(QBrush(Qt::red));
    cSym->setSize(5);

    QString cLegend = "calculated";
    //crvc[0] = insertCurve(cLegend);
    //setCurveSymbol(crvc[0], cSym);
    //setCurvePen(crvc[0], QPen("red",1));
    //setCurveYAxis(crvc[0], QwtPlot::yLeft);
    //setCurveData(crvc[0], cFS, cOF, ncFS);

    crvc[0] = new QwtPlotCurve(cLegend);
    crvc[0]->setRenderHint(QwtPlotItem::RenderAntialiased);
    crvc[0]->setPen(Qt::blue);
    crvc[0]->setSymbol(cSym);
    crvc[0]->setYAxis(QwtPlot::yLeft);
    crvc[0]->setSamples(cFS, cOF, ncFS);
    crvc[0]->attach(this);


    setAxisScale(QwtPlot::xBottom, xMin, xMax,0);

    // setAxisScale(QwtPlot::yLeft, yMin*0.95, yMax*1.05,0);
    yMin *= 0.9;
    yMax *= 1.1;
    setAxisScale(QwtPlot::yLeft, yMin, yMax,0);
    setAxisTitle(QwtPlot::yLeft, "Output Factor (cGy/MU)");
    setAxisTitle(QwtPlot::xBottom, "Field Size (mm)");
    QString PWD = getenv("PWD");
    QString mName = PWD.section('/',-2,-2);
    setTitle("Output Factor Comparison: " + mName);

    //long lHeadMarker = insertMarker();
    //setMarkerPos(lHeadMarker, xMin,yMin+(yMax-yMin));
    //setMarkerLabelAlign(lHeadMarker, Qt::AlignRight|Qt::AlignBottom);
    //setMarkerPen(lHeadMarker, QPen(Qt::blue, 0, Qt::DashDotLine));
    //setMarkerFont(lHeadMarker, QFont("Helvetica", 7, QFont::Normal));

    QwtPlotMarker *lHeadMarker = new QwtPlotMarker();
    lHeadMarker->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
    lHeadMarker->setLinePen(QPen(Qt::blue, 0, Qt::DashDotLine));
    //lHeadMarker->setLineStyle(QwtPlotMarker::VLine);
    lHeadMarker->setValue(xMin,yMin+(yMax-yMin));

    QString qsInfo = Info.section("Phantom:",0,0)
                     + " ISO:" + Info.section("ISO:",1,1).section("CPU:",0,0)
                     + "\n BDT:"+ Info.section("BDT:",1,1).section("Zs:",0,0)
                     + "\n Zs:"+ Info.section("Zs:",1,1);

    QwtText marker(qsInfo);
    QFont font("Helvetica", 7, QFont::Normal);
    marker.setFont(font);

    lHeadMarker->setLabel(marker);
    lHeadMarker->attach(this);

//    QwtPlotTextLabel *markerItem = new QwtPlotTextLabel();
//    markerItem->setText(marker);
//    markerItem->attach(this);

    // setMarkerLabelText(lHeadMarker,qsInfo);

    QString Info[100];
    //long titleMarker = insertMarker();
    //setMarkerPos(titleMarker, (xMax+xMin)/2.0,(yMax+yMin)/2.0+(yMax-yMin)/25);
    //setMarkerLabelAlign(titleMarker, Qt::AlignRight|Qt::AlignBottom);
    //setMarkerPen(titleMarker, QPen(Qt::blue, 0, Qt::DashDotLine));
    //setMarkerFont(titleMarker, QFont("Courier", 10, QFont::Normal));

    QwtPlotMarker *titleMarker = new QwtPlotMarker();
    titleMarker->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
    titleMarker->setLinePen(QPen(Qt::blue, 0, Qt::DashDotLine));
    //titleMarker->setLineStyle(QwtPlotMarker::VLine);
    titleMarker->setValue((xMax+xMin)/2.0,(yMax+yMin)/2.0+(yMax-yMin)/25);

    QString titleText = "";
    titleText.sprintf("Output Table at %3.0f mm depth",refDepth*10);
    QwtText titleTextMarker(titleText);
    QFont titleFont("Courier", 10, QFont::Normal);
    titleTextMarker.setFont(titleFont);

    titleMarker->setLabel(titleTextMarker);
    titleMarker->attach(this);

//    QwtPlotTextLabel *titleMarkerItem = new QwtPlotTextLabel();
//    titleMarkerItem->setText(titleTextMarker);
//    titleMarkerItem->attach(this);
    // setMarkerLabelText(titleMarker,titleText);

    //long tMarker = insertMarker();
    //setMarkerPos(tMarker, (xMax+xMin)/2.0,(yMax+yMin)/2.0);
    //setMarkerLabelAlign(tMarker, Qt::AlignRight|Qt::AlignBottom);
    //setMarkerPen(tMarker, QPen(Qt::blue, 0, Qt::DashDotLine));
    //setMarkerFont(tMarker, QFont("Courier", 10, QFont::Normal));

    QwtPlotMarker *tMarker = new QwtPlotMarker();
    tMarker->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
    tMarker->setLinePen(QPen(Qt::blue, 0, Qt::DashDotLine));
    //tMarker->setLineStyle(QwtPlotMarker::VLine);
    tMarker->setValue((xMax+xMin)/2.0,(yMax+yMin)/2.0);

    QString tText = "FS(mm)\t\tMeasured\t\tCalculated\t\t\% Diff";
    QwtText tTextMarker(tText);
    QFont tFont("Courier", 10, QFont::Normal);
    tTextMarker.setFont(tFont);

    tMarker->setLabel(tTextMarker);
    tMarker->attach(this);

//    QwtPlotTextLabel *tMarkerItem = new QwtPlotTextLabel();
//    tMarkerItem->setText(tTextMarker);
//    tMarkerItem->attach(this);

    // setMarkerLabelText(tMarker,tText);

    QwtPlotMarker *textMarker[100];
    for (int i=0; i<ncFS; i++)
    {
        //textMarker[i] = insertMarker();
        //setMarkerPos(textMarker[i], (xMax+xMin)/2.0,(yMax+yMin)/2.0-(yMax-yMin)/25*(i+1));
        //setMarkerLabelAlign(textMarker[i], Qt::AlignRight|Qt::AlignBottom);
        //setMarkerPen(textMarker[i], QPen(Qt::blue, 0, Qt::DashDotLine));
        //setMarkerFont(textMarker[i], QFont("Courier", 10, QFont::Normal));

        textMarker[i] = new QwtPlotMarker();
        textMarker[i]->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
        textMarker[i]->setLinePen(QPen(Qt::blue, 0, Qt::DashDotLine));
        textMarker[i]->setValue((xMax+xMin)/2.0,(yMax+yMin)/2.0-(yMax-yMin)/25*(i+1));


        float OF = 0.0;
        float DIFF = 100.0;
        for (int j=0; j<nmFS; j++)
        {
            if (mFS[j]==cFS[i])
            {
                OF=mOF[j];
                DIFF = (cOF[i] - OF)/OF*100.0;
            }
        }
        if (OF == 0)
            Info[i].sprintf("%5.0f\t\t   N/A  \t\t%8.3f\t\t   N/A  \n",cFS[i], cOF[i]);
        else
            Info[i].sprintf("%5.0f\t\t%8.3f\t\t%8.3f\t\t%8.3f\n",cFS[i], OF, cOF[i], DIFF);


        QwtText txtTextMarker(Info[i]);
        QFont txtFont("Courier", 10, QFont::Normal);
        txtTextMarker.setFont(txtFont);

        textMarker[i]->setLabel(txtTextMarker);
        textMarker[i]->attach(this);

//        QwtPlotTextLabel *txtMarkerItem = new QwtPlotTextLabel();
//        txtMarkerItem->setText(txtTextMarker);
//        txtMarkerItem->attach(this);

        // setMarkerLabelText(textMarker[i],Info[i]);
    }


    // replot();
    // updatePlot();
    // changeSymbol();
}

void MyPlotComp::updatePlot()
{
    setAxisScale(QwtPlot::xBottom, xMin, xMax,0);
    setAxisScale(QwtPlot::yLeft, yMin, yMax,0);
    replot();
}

