#ifndef PLOTCOMP_H
#define PLOTCOMP_H

#include <QMainWindow>

namespace Ui {
class PlotComp;
}

class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
class Plot;
class QPolygon;

class PlotComp : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlotComp(QWidget *parent = 0);
    ~PlotComp();

    void init();
    void resetZoomer();

    virtual void setDataDir( char * d );
    virtual void setFactor( char * f );
    virtual void loadData( char * dName );

private slots:
    void moved( const QPoint & );
    void selected( const QPolygon & );
    void runGammaTable();

#ifndef QT_NO_PRINTER
    void print();
#endif

    void exportDocument();
    void enableZoomMode( bool );

    virtual void Cancel();
    virtual void Done();
    virtual void getXPlotData();
    virtual void getYPlotData();
    virtual void getZPlotData();
    virtual void toggleSymbol();
    virtual void toggleFilter();
    virtual void setFilterPoint();
    virtual void showLegend();
    virtual void showDD();
    virtual void showAllCurves();
    virtual void updateInfo();
    virtual void pddNorm();
    virtual void getAllPDDs();
    virtual void getAllCalcPDDs();
    virtual void plotOutputFactor();
    virtual void xMinChange();
    virtual void xMaxChange();
    virtual void yMinChange();
    virtual void yMaxChange();
    virtual void printAll();


    void on_xListBox_currentRowChanged(int currentRow);
    void on_yListBox_currentRowChanged(int currentRow);
    void on_zListBox_currentRowChanged(int currentRow);

    void on_pushButtonDone_clicked();

    void on_radioButtonDD_toggled(bool checked);

    void on_radioButtonLegend_toggled(bool checked);

    void on_radioButtonFiltered_toggled(bool checked);

    void on_spinBoxOrder_valueChanged(int arg1);

    void on_radioButtonLine_toggled(bool checked);

    void on_radioButtonDot_toggled(bool checked);

    void on_pushButtonPDDs_clicked();

    void on_radioButtonNorm_toggled(bool checked);

    void on_pushButtonCalcPDDs_clicked();

    void on_pushButtonPrintAll_clicked();

    void on_pushButtonOF_clicked();

    void on_spinBoxDepth_valueChanged(int arg1);

    void on_lineEditXMax_textChanged(const QString &arg1);

    void on_lineEditXMin_textChanged(const QString &arg1);

    void on_lineEditYMax_textChanged(const QString &arg1);

    void on_lineEditYMin_textChanged(const QString &arg1);

    void on_radioButtonCalc_toggled(bool checked);

    void on_radioButtonMeas_toggled(bool checked);

private:
    void showInfo( QString text = QString::null );

    Ui::PlotComp *ui;

    QwtPlotZoomer *d_zoomer[2];
    QwtPlotPicker *d_picker;
    QwtPlotPanner *d_panner;
};

#endif // PLOTCOMP_H
