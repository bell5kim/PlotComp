#include <qregexp.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qstatusbar.h>
#include <qprinter.h>
#include <qpicture.h>
#include <qpainter.h>
#include <qprintdialog.h>

#include <qwt_counter.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_renderer.h>
#include <qwt_text.h>
#include <qwt_math.h>

#include <QDomElement>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>

#include "pixmaps.h"
#include "plotcomp.h"
#include "ui_plotcomp.h"

using namespace std;
#include <iostream>
#include <cmath>


class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer( int xAxis, int yAxis, QWidget *canvas ):
        QwtPlotZoomer( xAxis, yAxis, canvas )
    {
        setTrackerMode( QwtPicker::AlwaysOff );
        setRubberBand( QwtPicker::NoRubberBand );

        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

        setMousePattern( QwtEventPattern::MouseSelect2,
                         Qt::RightButton, Qt::ControlModifier );
        setMousePattern( QwtEventPattern::MouseSelect3,
                         Qt::RightButton );
    }
};

PlotComp::PlotComp(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PlotComp)
{
    ui->setupUi(this);


    const int margin = 5;
    ui->qwtPlot->setContentsMargins( margin, margin, margin, 0 );

    setContextMenuPolicy( Qt::NoContextMenu );

    d_zoomer[0] = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft,
                              ui->qwtPlot->canvas() );
    d_zoomer[0]->setRubberBand( QwtPicker::RectRubberBand );
    d_zoomer[0]->setRubberBandPen( QColor( Qt::green ) );
    d_zoomer[0]->setTrackerMode( QwtPicker::ActiveOnly );
    d_zoomer[0]->setTrackerPen( QColor( Qt::white ) );

    d_zoomer[1] = new Zoomer( QwtPlot::xTop, QwtPlot::yRight,
                              ui->qwtPlot->canvas() );

    d_panner = new QwtPlotPanner( ui->qwtPlot->canvas() );
    d_panner->setMouseButton( Qt::MidButton );

    d_picker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                                  QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                                  ui->qwtPlot->canvas() );
    d_picker->setStateMachine( new QwtPickerDragPointMachine() );
    d_picker->setRubberBandPen( QColor( Qt::green ) );
    d_picker->setRubberBand( QwtPicker::CrossRubberBand );
    d_picker->setTrackerPen( QColor( Qt::white ) );

    // setCentralWidget( ui->qwtPlot );

    QToolBar *toolBar = new QToolBar( this );

    QToolButton *btnZoom = new QToolButton( toolBar );
    btnZoom->setText( "Zoom" );
    btnZoom->setIcon( QPixmap( zoom_xpm ) );
    btnZoom->setCheckable( true );
    btnZoom->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( btnZoom );
    connect( btnZoom, SIGNAL( toggled( bool ) ), SLOT( enableZoomMode( bool ) ) );

#ifndef QT_NO_PRINTER
    QToolButton *btnPrint = new QToolButton( toolBar );
    btnPrint->setText( "Print" );
    btnPrint->setIcon( QPixmap( print_xpm ) );
    btnPrint->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( btnPrint );
    connect( btnPrint, SIGNAL( clicked() ), SLOT( print() ) );
#endif

    QToolButton *btnExport = new QToolButton( toolBar );
    btnExport->setText( "Export" );
    btnExport->setIcon( QPixmap( print_xpm ) );
    btnExport->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( btnExport );
    connect( btnExport, SIGNAL( clicked() ), SLOT( exportDocument() ) );

    QToolButton *btnTable = new QToolButton( toolBar );
    btnTable->setText( "Table" );
    btnTable->setIcon( QPixmap( table_xpm ) );
    btnTable->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( btnTable );
    connect( btnTable, SIGNAL( clicked() ), SLOT( runGammaTable() ) );

    /*
        toolBar->addSeparator();

        QWidget *hBox = new QWidget( toolBar );

        QHBoxLayout *layout = new QHBoxLayout( hBox );
        layout->setSpacing( 0 );
        layout->addWidget( new QWidget( hBox ), 10 ); // spacer
        layout->addWidget( new QLabel( "Damping Factor", hBox ), 0 );
        layout->addSpacing( 10 );

        QwtCounter *cntDamp = new QwtCounter( hBox );
        cntDamp->setRange( 0.0, 5.0 );
        cntDamp->setSingleStep( 0.01 );
        cntDamp->setValue( 0.0 );

        layout->addWidget( cntDamp, 0 );

        ( void )toolBar->addWidget( hBox );
    */

    addToolBar( toolBar );

#ifndef QT_NO_STATUSBAR
    ( void )statusBar();
#endif

    enableZoomMode( false );
    showInfo();

    //connect( cntDamp, SIGNAL( valueChanged( double ) ),
    //    ui->qwtPlot, SLOT( setDamp( double ) ) );

    // connect( cntDamp, &QwtCounter::valueChanged, damping, &Plot::damp );
    // connect( cntDamp, &QwtCounter::valueChanged, ui->qwtPlot, &Plot::damp );

    connect( d_picker, SIGNAL( moved( const QPoint & ) ),
             SLOT( moved( const QPoint & ) ) );

    connect( d_picker, SIGNAL( selected( const QPolygon & ) ),
             SLOT( selected( const QPolygon & ) ) );

    init();

}

PlotComp::~PlotComp()
{
    delete ui;
}


// CLASS Status ------------------------------------------------------
#define XML on
#ifdef XML
class Status
{
public:
    QString value, date;
};

QDomElement StatusToNode (QDomDocument &d, const Status &s, QString e)
{
    QDomElement elm = d.createElement(e);
    elm.setAttribute("value", s.value);
    elm.setAttribute("date", s.date);
    return elm;
};
#endif


#ifndef QT_NO_PRINTER

void PlotComp::print()
{
    QPrinter printer( QPrinter::HighResolution );

    QString docName = ui->qwtPlot->title().text();
    if ( !docName.isEmpty() )
    {
        docName.replace ( QRegExp ( QString::fromLatin1 ( "\n" ) ), tr ( " -- " ) );
        printer.setDocName ( docName );
    }

    printer.setCreator( "Air Fitting Report" );
    printer.setOrientation( QPrinter::Landscape );
    printer.setPageSize(QPrinter::Letter);
    printer.setColorMode(QPrinter::Color);

    QPrintDialog dialog( &printer );
    if ( dialog.exec() )
    {
        QwtPlotRenderer renderer;

        if ( printer.colorMode() == QPrinter::GrayScale )
        {
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
            renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
        }

        renderer.renderTo( ui->qwtPlot, printer );
    }
}

#endif

void PlotComp::exportDocument()
{
    QwtPlotRenderer renderer;
    renderer.exportTo( ui->qwtPlot, "PlotComp.pdf" );
}

void PlotComp::enableZoomMode( bool on )
{
    d_panner->setEnabled( on );

    d_zoomer[0]->setEnabled( on );
    d_zoomer[0]->zoom( 0 );

    d_zoomer[1]->setEnabled( on );
    d_zoomer[1]->zoom( 0 );

    d_picker->setEnabled( !on );

    showInfo();
}

void PlotComp::showInfo( QString text )
{
    if ( text == QString::null )
    {
        if ( d_picker->rubberBand() )
            text = "Cursor Pos: Press left mouse button in plot region";
        else
            text = "Zoom: Press mouse button and drag";
    }

#ifndef QT_NO_STATUSBAR
    statusBar()->showMessage( text );
#endif
}

void PlotComp::moved( const QPoint &pos )
{
    QString info;
    info.sprintf( "x=%g, Dose=%g, Diff=%g",
                  ui->qwtPlot->invTransform( QwtPlot::xBottom, pos.x() ),
                  ui->qwtPlot->invTransform( QwtPlot::yLeft, pos.y() ),
                  ui->qwtPlot->invTransform( QwtPlot::yRight, pos.y() )
                );
    showInfo( info );
}

void PlotComp::selected( const QPolygon & )
{
    showInfo();
}

void PlotComp::resetZoomer()
{
    d_zoomer[0]->setZoomBase();
    d_zoomer[1]->setZoomBase();
}


void PlotComp::runGammaTable()
{

    QString LBIN;
    LBIN = getenv("XVMC_HOME");

    QString CMD = LBIN + "/gammatable.exe -i ";
    QDir *dirCalc = new QDir;
    dirCalc->setPath(".tmp");

    QString FNAME(ui->qwtPlot->fname);

    QString profile = "";
    if (FNAME.contains(".px")) profile = "x";
    if (FNAME.contains(".py")) profile = "y";
    if (FNAME.contains(".pz")) profile = "z";

    QStringList filefilter;
    filefilter << FNAME.section('.',0,2);
    // cout << "FNAME = " << FNAME << endl;


    if (FNAME.contains(".pz"))
    {
        filefilter << "."+profile+"Comp";
        dirCalc->setNameFilters(filefilter);
    }
    else
    {
        filefilter << "*."+profile+"Comp";
        dirCalc->setNameFilters(filefilter);
    }
    QStringList cStrList = dirCalc->entryList(QDir::Files, QDir::Name);
    cStrList.sort();
    for ( QStringList::Iterator it = cStrList.begin(); it != cStrList.end(); ++it )
    {
        QString FN(*it);
        QTextStream(&CMD) << CMD << ".tmp/" << FN << ":";
    }

    // cout << CMD << endl;
    if (system(CMD.toStdString().c_str()) != 0)
    {
        cout << "ERROR: Somethings are wrong: " << CMD.toStdString() << '\n';
    }
}



void PlotComp::init()
{
    QString XVMC_WORK = getenv("XVMC_WORK");
    QString mName = XVMC_WORK.section('/',-1);
    ui->textLabelMachine->setText(mName);
    ui->qwtPlot->setWindowTitle("Verification of Monte Carlo to Measurement for " + mName);
    // Show Tool Bar on the main plot area
    // ui->qwtPlot->showToolBar(this);
    ui->qwtPlot->SymbolSwitch="Line";
    ui->qwtPlot->FilterSwitch="Raw";

    setFilterPoint();
}


void PlotComp::setDataDir(char *d)
{
    strcpy(ui->qwtPlot->dname, d);
}

void PlotComp::setFactor(char *f)
{
    QString factor = f;
    bool ok;
    ui->qwtPlot->factor = factor.toFloat(&ok);
}

void PlotComp::Cancel()
{
    close();
}

void PlotComp::Done()
{
    close();
}


void PlotComp::getXPlotData()
{
    strcpy(ui->qwtPlot->fname,ui->xListBox->currentItem()->text().replace("0.px", ".px").toLatin1());
    //QTextStream (stdout) << "getXPlotData: xListBox->text = " << ui->xListBox->currentItem()->text() << endl;
    //QTextStream (stdout) << "getXPlotData: dname = " << ui->qwtPlot->dname << endl;
    //QTextStream (stdout) << "getXPlotData: fname = " << ui->qwtPlot->fname << endl;
    ui->qwtPlot->usrDataFile(ui->qwtPlot->dname, ui->qwtPlot->fname);
    resetZoomer();
    // QTextStream (stdout) << "getXPlotData:1 " << endl;
// d_plot->replot();
    if (ui->yListBox->currentRow() > -1)
        ui->yListBox->item(ui->yListBox->currentRow())->setSelected(false);
    if (ui->zListBox->currentRow() > -1)
        ui->zListBox->item(ui->zListBox->currentRow())->setSelected(false);
    /*
    if (ui->yListBox->currentRow() > -1)
    {

        QTextStream (stdout) << "getXPlotData:1 1 " << endl;
      QStringList yStrListFinal;
      QTextStream (stdout) << "getXPlotData:1 2 " << endl;
      for (unsigned int j=0; j<ui->yListBox->count(); j++)
        yStrListFinal << ui->yListBox->item(j)->text();

      QTextStream (stdout) << "getXPlotData:1 3 " << endl;
      //ui->yListBox->clear();
      QTextStream (stdout) << "getXPlotData:1 4 " << endl;
      ui->yListBox->insertItems(-1,yStrListFinal);
      QTextStream (stdout) << "getXPlotData:1 5 " << endl;
    }
    QTextStream (stdout) << "getXPlotData:2 " << endl;
    if (ui->zListBox->currentRow() > -1)
    {
      QStringList zStrListFinal;
      for (unsigned int k=0; k<ui->zListBox->count(); k++)
        zStrListFinal << ui->zListBox->item(k)->text();
      //ui->zListBox->clear();
      ui->zListBox->insertItems(-1, zStrListFinal);
    }
    */
    toggleFilter();
    toggleSymbol();
    showLegend();
    updateInfo();
    ui->radioButtonMeas->setChecked(true);
    ui->radioButtonCalc->setChecked(true);
}

void PlotComp::getYPlotData()
{
    strcpy(ui->qwtPlot->fname,ui->yListBox->currentItem()->text().replace("0.py", ".py").toLatin1());
    // cout << ui->yListBox->currentText() << endl;
    ui->qwtPlot->usrDataFile(ui->qwtPlot->dname, ui->qwtPlot->fname);

    resetZoomer();

// d_plot->replot();

    if (ui->xListBox->currentRow() > -1)
        ui->xListBox->item(ui->xListBox->currentRow())->setSelected(false);
    if (ui->zListBox->currentRow() > -1)
        ui->zListBox->item(ui->zListBox->currentRow())->setSelected(false);
    /*
    if (ui->xListBox->currentRow() > -1)
    {
    QStringList xStrListFinal;
    for (unsigned int i=0; i<ui->xListBox->count(); i++)
     xStrListFinal << ui->xListBox->item(i)->text();
    //ui->xListBox->clear();
      ui->xListBox->insertItems(-1, xStrListFinal);
    }
    if (ui->zListBox->currentRow() > -1)
    {
    QStringList zStrListFinal;
    for (unsigned int k=0; k<ui->zListBox->count(); k++)
     zStrListFinal << ui->zListBox->item(k)->text();
    //ui->zListBox->clear();
      ui->zListBox->insertItems(-1, zStrListFinal);
    }
    */
    toggleFilter();
    toggleSymbol();
    showLegend();
    updateInfo();
    ui->radioButtonMeas->setChecked(true);
    ui->radioButtonCalc->setChecked(true);
}
void PlotComp::getZPlotData()
{
    strcpy(ui->qwtPlot->fname,ui->zListBox->currentItem()->text().replace("0.pz", ".pz").toLatin1());
    // cout << ui->zListBox->currentText() << endl;
    ui->qwtPlot->usrDataFile(ui->qwtPlot->dname, ui->qwtPlot->fname);

    resetZoomer();

// d_plot->replot();

    if (ui->xListBox->currentRow() > -1)
        ui->xListBox->item(ui->xListBox->currentRow())->setSelected(false);
    if (ui->yListBox->currentRow() > -1)
        ui->yListBox->item(ui->yListBox->currentRow())->setSelected(false);
    /*
     if (ui->xListBox->currentRow() > -1)
     {
       QStringList xStrListFinal;
       for (unsigned int i=0; i<ui->xListBox->count(); i++)
        xStrListFinal << ui->xListBox->item(i)->text();
       //ui->xListBox->clear();
         ui->xListBox->insertItems(-1,xStrListFinal);
       }
     if (ui->yListBox->currentRow() > -1)
     {
       QStringList yStrListFinal;
       for (unsigned int j=0; j<ui->yListBox->count(); j++)
        yStrListFinal << ui->yListBox->item(j)->text();
       //ui->yListBox->clear();
         ui->yListBox->insertItems(-1,yStrListFinal);
       }
     */
    toggleFilter();
    toggleSymbol();
    showLegend();
    updateInfo();
    ui->radioButtonMeas->setChecked(true);
    ui->radioButtonCalc->setChecked(true);
}

void PlotComp::toggleSymbol()
{
    if (ui->radioButtonDot->isChecked())
    {
        ui->qwtPlot->SymbolSwitch="Dot";
    }
    else
    {
        ui->qwtPlot->SymbolSwitch="Line";
    }

    ui->qwtPlot->changeSymbol();
}

void PlotComp::toggleFilter()
{
    if (ui->radioButtonFiltered->isChecked())
    {
        ui->qwtPlot->FilterSwitch="Filtered";
    }
    else
    {
        ui->qwtPlot->FilterSwitch="Raw";
    }
    ui->qwtPlot->smoothing();
}

void PlotComp::setFilterPoint()
{
    ui->qwtPlot->nFilterPoints = ui->spinBoxOrder->value();
    toggleFilter();
}

void PlotComp::showLegend()
{
    if (ui->radioButtonLegend->isChecked())
    {
        ui->qwtPlot->LegendSwitch="Show";
    }
    else
    {
        ui->qwtPlot->LegendSwitch="Hide";
    }
    ui->qwtPlot->showLegend();
}


void PlotComp::showDD()
{
    if (ui->radioButtonDD->isChecked())
    {
        ui->qwtPlot->DDSwitch="Show";
    }
    else
    {
        ui->qwtPlot->DDSwitch="Hide";
    }
    ui->qwtPlot->showDD();
}


void PlotComp::showAllCurves()
{
    if (ui->radioButtonMeas->isChecked())
    {
        ui->qwtPlot->MeasSwitch="On";
    }
    else
    {
        ui->qwtPlot->MeasSwitch="Off";
    }

    if (ui->radioButtonCalc->isChecked())
    {
        ui->qwtPlot->CalcSwitch="On";
    }
    else
    {
        ui->qwtPlot->CalcSwitch="Off";
    }

    ui->qwtPlot->showAllCurves();
}

void PlotComp::updateInfo()
{
    QString Info = ui->qwtPlot->Info;
    QString Date = Info.section("Date:",1,1).section(" ",0,2);
    ui->lineEditDate->setText(Date);

    QString Time = Info.section("Date:",1,1).section(" ",3,3);
    ui->lineEditTime->setText(Time);

    QString CPU = Info.section("CPU:",1,1).section(" ",0,0);
    if (CPU.length() == 0) CPU = Info.section("CPU:",1,1).section(" ",1,1);
    ui->lineEditCPU->setText(CPU);

    QString Phantom = Info.section("Phantom:",1,1).section("(",0,0);
    ui->lineEditPhantom->setText(Phantom);

    QString Voxel = Info.section("Phantom:",1,1).section("(",1,1).section(")",0,0);
    ui->lineEditVoxel->setText(Voxel);

    QString SSD = Info.section("SSD:",1,1).section(" ",2,2);
    ui->lineEditSSD->setText(SSD);

    QString Pp = Info.section("Pp:",1,1).section(" ",0,0).section("%",0,0);
    ui->lineEditPp->setText(Pp);

    QString Ps = Info.section("Ps:",1,1).section(" ",0,0).section("%",0,0);
    ui->lineEditPs->setText(Ps);

    QString Sp = Info.section("Sp:",1,1).section(" ",0,0).section("cm",0,0);
    ui->lineEditSp->setText(Sp);

    QString Ss = Info.section("Ss:",1,1).section(" ",0,0).section("cm",0,0);
    ui->lineEditSs->setText(Ss);

    QString Pe = Info.section("Pe:",1,1).section(" ",0,0).section("%",0,0);
    ui->lineEditPe->setText(Pe);

    QString Emin = Info.section("Emin:",1,1).section(" ",0,0).section("MeV",0,0);
    ui->lineEditEmin->setText(Emin);

    QString Emax = Info.section("Emax:",1,1).section(" ",0,0).section("MeV",0,0);
    ui->lineEditEmax->setText(Emax);

    QString L = Info.section("L:",1,1).section(" ",0,0);
    ui->lineEditL->setText(L);

    QString B = Info.section("B:",1,1).section(" ",0,0);
    ui->lineEditB->setText(B);

    QString A = Info.section("A:",1,1).section(" ",0,0);
    ui->lineEditA->setText(A);

    QString Z = Info.section("Z:",1,1).section(" ",0,0);
    ui->lineEditZval->setText(Z);

    QString Nu = Info.section("Nu:",1,1).section(" ",0,0);
    ui->lineEditNu->setText(Nu);

}

void PlotComp::pddNorm()
{
    if (ui->radioButtonNorm->isChecked())
    {
        ui->qwtPlot->NormSwitch="Norm";
    }
    else
    {
        ui->qwtPlot->NormSwitch="Raw";
    }
    ui->qwtPlot->pddNorm();
}

void PlotComp::getAllPDDs()
{
    ui->qwtPlot->previewPDD("meas", "*.mz0");
    resetZoomer();
    showLegend();
}

void PlotComp::getAllCalcPDDs()
{
    ui->qwtPlot->previewPDD("calc", "*.pz0");
    resetZoomer();
    showLegend();
}


void PlotComp::plotOutputFactor()
{
    QString Depth = ui->spinBoxDepth->text();
    bool ok;
    float refDepth = Depth.toFloat(&ok);
    ui->qwtPlot->plotOF("meas", "*.mz0", ui->qwtPlot->dname, "*.pz0", refDepth);
    resetZoomer();
    QString xMaxStr="";
    xMaxStr.sprintf("%8.0f", ui->qwtPlot->xMax);
    ui->lineEditXMax->setText(xMaxStr);
    QString xMinStr="";
    xMinStr.sprintf("%8.0f", ui->qwtPlot->xMin);
    ui->lineEditXMin->setText(xMinStr);
    QString yMaxStr="";
    yMaxStr.sprintf("%8.3f", ui->qwtPlot->yMax);
    ui->lineEditYMax->setText(yMaxStr);
    QString yMinStr="";
    yMinStr.sprintf("%8.3f", ui->qwtPlot->yMin);
    ui->lineEditYMin->setText(yMinStr);
    showLegend();
    xMinChange();
    xMaxChange();
    yMinChange();
    yMaxChange();
}

void PlotComp::xMinChange()
{
    bool ok;
    QString xMinValue = ui->lineEditXMin->text();
    float xMin = xMinValue.toFloat(&ok);
    ui->qwtPlot->xMin=xMin;
    ui->qwtPlot->updatePlot();
}

void PlotComp::xMaxChange()
{
    bool ok;
    QString xMaxValue = ui->lineEditXMax->text();
    float xMax = xMaxValue.toFloat(&ok);
    ui->qwtPlot->xMax=xMax;
    ui->qwtPlot->updatePlot();
}

void PlotComp::yMinChange()
{
    bool ok;
    QString yMinValue = ui->lineEditYMin->text();
    float yMin = yMinValue.toFloat(&ok);
    ui->qwtPlot->yMin=yMin;
    ui->qwtPlot->updatePlot();
}

void PlotComp::yMaxChange()
{
    bool ok;
    QString yMaxValue = ui->lineEditYMax->text();
    float yMax = yMaxValue.toFloat(&ok);
    ui->qwtPlot->yMax=yMax;
    ui->qwtPlot->updatePlot();
}

void PlotComp::printAll()
{
    ui->radioButtonLegend->setChecked(true);

    QString PWD = getenv("PWD");
    QString MTD = "/MCvsMeas_";
    if ( PWD.section('/',-1) == "PB") MTD = "/PBvsMeas_";
    QString PS = "PS";
    QString PATH = PWD.section('/',0,-2);
    QDir *psDir = new QDir;
    psDir->setPath(PATH+"/"+PS);
    if(!psDir->exists())
    {
        psDir->setPath(PATH);
        psDir->mkdir(PS);
    }

    int nX = ui->xListBox->count();
    int nY = ui->yListBox->count();
    int nZ = ui->zListBox->count();
    for (int i=0; i< nX; i++)
    {
        ui->xListBox->setCurrentRow(i);
        QString xName = ui->xListBox->item(i)->text();
        QString fName = "../"+PS+MTD+xName+".ps";
        // cout << fName << endl;
        ui->qwtPlot->printPS(fName);
    }

    for (int i=0; i< nY; i++)
    {
        ui->yListBox->setCurrentRow(i);
        QString yName = ui->yListBox->item(i)->text();
        QString fName = "../"+PS+MTD+yName+".ps";
        // cout << fName << endl;
        ui->qwtPlot->printPS(fName);
    }

    for (int i=0; i< nZ; i++)
    {
        ui->zListBox->setCurrentRow(i);
        QString zName = ui->zListBox->item(i)->text();
        QString fName = "../"+PS+MTD+zName+".ps";
        // cout << fName << endl;
        ui->qwtPlot->printPS(fName);
    }

    plotOutputFactor();
    QString qsFileName = "../"+PS+MTD+"OF.ps";
    ui->qwtPlot->printPS(qsFileName);
}

void PlotComp::loadData(char *dName)
{
    QDir *dirCalc = new QDir;
    dirCalc->setPath(dName);

    dirCalc->setNameFilters(QStringList("*.px*"));
    QStringList xStrList = dirCalc->entryList(QDir::Files, QDir::Name);
    QString xPrevious("");
    QString xEntry("");
    QStringList xStrList5d;
    QStringList xStrList6d;
    QStringList xStrList7d;
    // QStringList xStrListFinal;
    for ( QStringList::Iterator it = xStrList.begin(); it != xStrList.end(); ++it )
    {
        QString tst(*it);
        xEntry = tst.section('.',1,1);
        QString mFileName = tst.replace("px", "mx");
        if (xEntry != xPrevious && QFile::exists("meas/" + mFileName))
        {
            xPrevious = xEntry;
            xEntry.prepend('.');
            xEntry.prepend(tst.section('.',0,0));
            xEntry.append('.');
            xEntry.append(tst.section('.',2,2));
            xEntry.append("0.px");
            if (xPrevious.length() == 5) xStrList5d << xEntry;
            if (xPrevious.length() == 6) xStrList6d << xEntry;
            if (xPrevious.length() == 7) xStrList7d << xEntry;
            // xStrListFinal << xEntry;
        }
        // cout << tst.section('.',1,1) << endl;
    }
    ui->xListBox->clear();
    ui->xListBox->addItems(xStrList5d);
    ui->xListBox->addItems(xStrList6d);
    ui->xListBox->addItems(xStrList7d);
    // xListBox->addItems(xStrListFinal, -1);

    dirCalc->setNameFilters(QStringList("*.py*"));
    QStringList yStrList = dirCalc->entryList(QDir::Files, QDir::Name);
    QString yPrevious("");
    QString yEntry("");
    QStringList yStrList5d;
    QStringList yStrList6d;
    QStringList yStrList7d;
    // QStringList yStrListFinal;
    for ( QStringList::Iterator it = yStrList.begin(); it != yStrList.end(); ++it )
    {
        QString tst(*it);
        yEntry = tst.section('.',1,1);
        QString mFileName = tst.replace("py", "my");
        if (yEntry != yPrevious && QFile::exists("meas/" + mFileName))
        {
            yPrevious = yEntry;
            yEntry.prepend('.');
            yEntry.prepend(tst.section('.',0,0));
            yEntry.append('.');
            yEntry.append(tst.section('.',2,2));
            yEntry.append("0.py");
            if (yPrevious.length() == 5) yStrList5d << yEntry;
            if (yPrevious.length() == 6) yStrList6d << yEntry;
            if (yPrevious.length() == 7) yStrList7d << yEntry;
            // yStrListFinal << yEntry;
        }
    }
    ui->yListBox->clear();
    ui->yListBox->addItems(yStrList5d);
    ui->yListBox->addItems(yStrList6d);
    ui->yListBox->addItems(yStrList7d);
    // yListBox->addItems(yStrListFinal, -1);

    dirCalc->setNameFilters(QStringList("*.pz*"));
    QStringList zStrList = dirCalc->entryList(QDir::Files, QDir::Name);
    QString zPrevious("");
    QString zEntry("");
    QStringList zStrList5d;
    QStringList zStrList6d;
    QStringList zStrList7d;
    // QStringList zStrListFinal;
    for ( QStringList::Iterator it = zStrList.begin(); it != zStrList.end(); ++it )
    {
        QString tst(*it);
        zEntry = tst.section('.',1,1);
        QString mFileName = tst.replace("pz", "mz");
        if (zEntry != zPrevious && QFile::exists("meas/" + mFileName))
        {
            zPrevious = zEntry;
            zEntry.prepend('.');
            zEntry.prepend(tst.section('.',0,0));
            zEntry.append('.');
            zEntry.append(tst.section('.',2,2));
            zEntry.append("0.pz");
            if (zPrevious.length() == 5) zStrList5d << zEntry;
            if (zPrevious.length() == 6) zStrList6d << zEntry;
            if (zPrevious.length() == 7) zStrList7d << zEntry;
            // zStrListFinal << zEntry;
        }
    }
    ui->zListBox->clear();
    ui->zListBox->addItems(zStrList5d);
    ui->zListBox->addItems(zStrList6d);
    ui->zListBox->addItems(zStrList7d);
    // zListBox->addItems(zStrListFinal, -1);

    // Set Initial Item
    if (ui->zListBox->count() > 0) ui->zListBox->setCurrentItem(0);
    plotOutputFactor();
}

void PlotComp::on_xListBox_currentRowChanged(int currentRow)
{
    getXPlotData();
}

void PlotComp::on_yListBox_currentRowChanged(int currentRow)
{
    getYPlotData();
}

void PlotComp::on_zListBox_currentRowChanged(int currentRow)
{
    getZPlotData();
}


void PlotComp::on_pushButtonDone_clicked()
{
    Done();
}

void PlotComp::on_radioButtonDD_toggled(bool checked)
{
    showDD();
}

void PlotComp::on_radioButtonLegend_toggled(bool checked)
{
    showLegend();
}

void PlotComp::on_radioButtonFiltered_toggled(bool checked)
{
    toggleFilter();
}

void PlotComp::on_spinBoxOrder_valueChanged(int arg1)
{
    setFilterPoint();
}

void PlotComp::on_radioButtonLine_toggled(bool checked)
{
    toggleSymbol();
}

void PlotComp::on_radioButtonDot_toggled(bool checked)
{
    toggleSymbol();
}

void PlotComp::on_pushButtonPDDs_clicked()
{
    getAllPDDs();
}

void PlotComp::on_radioButtonNorm_toggled(bool checked)
{
    pddNorm();
}

void PlotComp::on_pushButtonCalcPDDs_clicked()
{
    getAllCalcPDDs();
}

void PlotComp::on_pushButtonPrintAll_clicked()
{
    printAll();
}

void PlotComp::on_pushButtonOF_clicked()
{
    plotOutputFactor();
}

void PlotComp::on_spinBoxDepth_valueChanged(int arg1)
{
    plotOutputFactor();
}

void PlotComp::on_lineEditXMax_textChanged(const QString &arg1)
{
    xMaxChange();
}

void PlotComp::on_lineEditXMin_textChanged(const QString &arg1)
{
    xMinChange();
}

void PlotComp::on_lineEditYMax_textChanged(const QString &arg1)
{
    yMaxChange();
}

void PlotComp::on_lineEditYMin_textChanged(const QString &arg1)
{
    yMinChange();
}

void PlotComp::on_radioButtonCalc_toggled(bool checked)
{
    showAllCurves();
}

void PlotComp::on_radioButtonMeas_toggled(bool checked)
{
    showAllCurves();
}
