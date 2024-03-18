#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <complex>
#include <vector>

namespace Ui {
class MainWindow;
}

typedef std::vector<std::vector<std::complex<double> > > DFTResult;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_actionDo_triggered();

    void on_actionInvDo_triggered();

    void on_actionSave_triggered();

private:
    Ui::MainWindow *ui;
    QImage imageOrigin;
    DFTResult dftResult;
    DFTResult fftResult;
    QImage inverseFFTResultImage;
    DFTResult perform2DFFT(const QImage& image);
    QImage visualizeDFT(const DFTResult& complexImage);
    QImage performInverse2DFFT(const DFTResult& dftResult);
    DFTResult applyLowPassFilterPixel(DFTResult& dftResult, double pixelRadius);
    DFTResult applyHighPassFilterPixel(DFTResult& dftResult, double pixelRadius);
    QImage complexImageToQImage(const DFTResult& complexImage);

};

#endif // MAINWINDOW_H
