#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <cmath>
#include <QVBoxLayout>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл", "", "Images (*.bmp)");
    if (!fileName.isEmpty()) {
        QDialog *imageDialog = new QDialog(this); // Создаем новый объект окна

        QLabel *newLabel = new QLabel(imageDialog); // Создаем новый QLabel на окне

        newLabel->setPixmap(QPixmap(fileName)); // Устанавливаем изображение в метку
        newLabel->resize(newLabel->pixmap()->size()); // Меняем размер метки под размер изображения

        imageOrigin = newLabel->pixmap()->toImage();

        QVBoxLayout *layout = new QVBoxLayout(imageDialog); // Создаем вертикальный макет для окна
        layout->addWidget(newLabel); // Добавляем метку на макет
        imageDialog->setLayout(layout); // Устанавливаем макет в качестве макета окна

        imageDialog->show(); // Показываем окно
    }
}


// Функция для применения фильтра нижних частот
DFTResult MainWindow::applyLowPassFilterPixel(DFTResult& dftResult, double pixelRadius) {
    int height = dftResult.size();
    int width = dftResult[0].size();

    // Создаем новый объект DFTResult для хранения результата
    DFTResult filteredResult(height, std::vector<std::complex<double>>(width));

    // Вычисляем центр изображения
    int centerX = width / 2;
    int centerY = height / 2;

    // Создаем временный массив для центрирования частот
    DFTResult centeredResult(height, std::vector<std::complex<double>>(width));

    // Центрирование низких частот
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int newX = (x + centerX) % width;
            int newY = (y + centerY) % height;
            centeredResult[newY][newX] = dftResult[y][x];
        }
    }

    // Применение фильтра
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Вычисляем расстояние от текущей точки до центра
            double distance = std::sqrt(std::pow(x - centerX, 2) + std::pow(y - centerY, 2));

            // Если расстояние меньше или равно порогу в пикселях, копируем значение из центрированного преобразования Фурье
            // В противном случае устанавливаем значение в 0
            if (distance <= pixelRadius) {
                filteredResult[y][x] = centeredResult[y][x];
            } else {
                filteredResult[y][x] = std::complex<double>(0.0, 0.0);
            }
        }
    }

    // Возвращаем частоты на исходное место
    DFTResult uncenteredResult(height, std::vector<std::complex<double>>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int newX = (x - centerX + width) % width;
            int newY = (y - centerY + height) % height;
            uncenteredResult[y][x] = filteredResult[newY][newX];
        }
    }

    return uncenteredResult;
}



DFTResult MainWindow::applyHighPassFilterPixel(DFTResult& dftResult, double pixelRadius) {
    int height = dftResult.size();
    int width = dftResult[0].size();

    // Создаем новый объект DFTResult для хранения результата
    DFTResult filteredResult(height, std::vector<std::complex<double>>(width));

    // Вычисляем центр изображения
    int centerX = width / 2;
    int centerY = height / 2;

    // Создаем временный массив для центрирования частот
    DFTResult centeredResult(height, std::vector<std::complex<double>>(width));

    // Центрирование низких частот
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int newX = (x + centerX) % width;
            int newY = (y + centerY) % height;
            centeredResult[newY][newX] = dftResult[y][x];
        }
    }

    // Применение фильтра
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Вычисляем расстояние от текущей точки до центра
            double distance = std::sqrt(std::pow(x - centerX, 2) + std::pow(y - centerY, 2));

            // Если расстояние больше порога в пикселях, копируем значение из центрированного преобразования Фурье
            // и масштабируем яркость
            if (distance > pixelRadius) {
                filteredResult[y][x] = centeredResult[y][x];
            } else {
                // В противном случае устанавливаем значение в 0
                filteredResult[y][x] = std::complex<double>(0.0, 0.0);
            }
        }
    }

    // Возвращаем частоты на исходное место
    DFTResult uncenteredResult(height, std::vector<std::complex<double>>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int newX = (x - centerX + width) % width;
            int newY = (y - centerY + height) % height;
            uncenteredResult[y][x] = filteredResult[newY][newX];
        }
    }

    return uncenteredResult;
}


void MainWindow::on_actionDo_triggered()
{
    if (imageOrigin.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No image loaded."));
        return;
    }

    // Получаем размеры изображения
//    int width = imageOrigin.width();
//    int height = imageOrigin.height();

    // Создаем новое изображение для результата прямого преобразования Фурье
    fftResult = perform2DFFT(imageOrigin);
    QImage fftResultImage = visualizeDFT(fftResult);
//    QImage fftResultImage = complexImageToQImage(fftResult);

    // Отображаем результат прямого преобразования в новом диалоговом окне
    QDialog *fftImageDialog = new QDialog(this);
    QLabel *fftLabel = new QLabel(fftImageDialog);
    fftLabel->setPixmap(QPixmap::fromImage(fftResultImage));
    fftLabel->resize(fftLabel->pixmap()->size());

    QVBoxLayout *fftLayout = new QVBoxLayout(fftImageDialog);
    fftLayout->addWidget(fftLabel);
    fftImageDialog->setLayout(fftLayout);
    fftImageDialog->show();

//    DFTResult filteredResult = applyLowPassFilterPixel(fftResult, 180.0);
//    DFTResult filteredResult = applyHighPassFilterPixel(fftResult, 20.0);
//    DFTResult filteredResult = applyFrequencyFilter(fftResult, 170.0, NAN);


}


vector<complex<double> > perform1DFFT(const vector<complex<double> >& input) {
    int N = input.size();
    vector<complex<double> > output(N);

    for (int k = 0; k < N; ++k) {
        complex<double> sum(0.0, 0.0);
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            complex<double> complexExp(cos(angle), sin(angle));
            sum += input[n] * complexExp;
        }
        output[k] = sum  / complex<double>(N, 0);
    }

    return output;
}


DFTResult MainWindow::perform2DFFT(const QImage& image) {
    int width = image.width();
    int height = image.height();

    // Преобразование каждого пикселя в комплексное число
    DFTResult complexImage(height, vector<complex<double> >(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QRgb pixel = image.pixel(x, y);
            complexImage[y][x] = complex<double>(qRed(pixel), 0);
        }
    }

    // Прямое преобразование Фурье для каждой строки
    for (int y = 0; y < height; ++y) {
        vector<complex<double> > row = complexImage[y];
        vector<complex<double> > transformedRow = perform1DFFT(row);
        for (int x = 0; x < width; ++x) {
            complexImage[y][x] = transformedRow[x];
        }
    }

    // Прямое преобразование Фурье для каждого столбца
    for (int x = 0; x < width; ++x) {
        vector<complex<double> > column;
        for (int y = 0; y < height; ++y) {
            column.push_back(complexImage[y][x]);
        }
        vector<complex<double> > transformedColumn = perform1DFFT(column);
        for (int y = 0; y < height; ++y) {
            complexImage[y][x] = transformedColumn[y];
        }
    }
    
    return complexImage;
}



vector<complex<double> > performInverse1DFFT(const vector<complex<double> >& input) {
    int N = input.size();
    vector<complex<double> > output(N);

    for (int n = 0; n < N; ++n) {
        complex<double> sum(0.0, 0.0);
        for (int k = 0; k < N; ++k) {
            double angle = 2.0 * M_PI * k * n / N;
            complex<double> complexExp(cos(angle), sin(angle));
            sum += input[k] * complexExp;
        }
        output[n] = sum;
    }

    return output;
}

QImage MainWindow::visualizeDFT(const DFTResult& complexImage) {
    int width = imageOrigin.width();
    int height = imageOrigin.height();   
    QImage resultImage(width, height, QImage::Format_RGB32);
    
    // Нормализация амплитуд и перенос нулевой частоты
    double maxAmplitude = 0.0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double amplitude = abs(complexImage[y][x]);
            if (amplitude > maxAmplitude) {
                maxAmplitude = amplitude;
            }
        }
    }
    
    const double brightnessFactor = 20.0; // Увеличение яркости
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double amplitude = abs(complexImage[y][x]);
            // Нормализация амплитуды
            amplitude = 255.0 * log(1.0 + amplitude) / log(1.0 + maxAmplitude);
            amplitude *= brightnessFactor;
            // Перенос нулевой частоты
            double shiftedAmplitude = amplitude;
            int shiftX = (x + width / 2) % width;
            int shiftY = (y + height / 2) % height;
            resultImage.setPixel(shiftX, shiftY, qRgb(shiftedAmplitude, shiftedAmplitude, shiftedAmplitude));
        }
    }
    
    return resultImage;    
}

QImage MainWindow::performInverse2DFFT(const DFTResult& dftResult) {
    int width = imageOrigin.width();
    int height = imageOrigin.height();

    QImage resultImage(width, height, QImage::Format_RGB32);
    DFTResult complexImage = dftResult;

    // Обратное преобразование Фурье для каждой строки
    for (int y = 0; y < height; ++y) {
        vector<complex<double> > row = complexImage[y];
        vector<complex<double> > transformedRow = performInverse1DFFT(row);
        for (int x = 0; x < width; ++x) {
            complexImage[y][x] = transformedRow[x];
        }
    }

    // Обратное преобразование Фурье для каждого столбца
    for (int x = 0; x < width; ++x) {
        vector<complex<double> > column;
        for (int y = 0; y < height; ++y) {
            column.push_back(complexImage[y][x]);
        }
        vector<complex<double> > transformedColumn = performInverse1DFFT(column);
        for (int y = 0; y < height; ++y) {
            complexImage[y][x] = transformedColumn[y];
        }
    }


    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double magnitude = abs(complexImage[y][x]);
            // Нормализация амплитуды
            magnitude = complexImage[y][x].real();
            // Ограничение значений пикселей в диапазоне [0, 255]
            int color = qBound(0, static_cast<int>(magnitude), 255);
            resultImage.setPixel(x, y, qRgb(color, color, color));
        }
    }

    return resultImage;
}



void MainWindow::on_actionInvDo_triggered()
{
    if (fftResult.empty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No FFT image loaded."));
        return;
    }
    // Выполняем обратное преобразование Фурье

    DFTResult filteredResult;
    if (ui->rbNoFilter->isChecked()) {
        inverseFFTResultImage = performInverse2DFFT(fftResult);
    }
    if (ui->rbLowFilter->isChecked()) {
        double parR = ui->sbLow->value();
        filteredResult = applyLowPassFilterPixel(fftResult, parR);
        inverseFFTResultImage = performInverse2DFFT(filteredResult);
    }
    if (ui->rbHighFilter->isChecked()) {
        double parR = ui->sbHigh->value();
        filteredResult = applyHighPassFilterPixel(fftResult, parR);
        inverseFFTResultImage = performInverse2DFFT(filteredResult);
    }

    if ((ui->rbHighFilter->isChecked()) or (ui->rbLowFilter->isChecked())) {
        QImage fftResultImageFiltred = visualizeDFT(filteredResult);

        // Отображаем результат применения фильтра в новом диалоговом окне
        QDialog *fftImageFiltredDialog = new QDialog(this);
        QLabel *fftFilterLabel = new QLabel(fftImageFiltredDialog);
        fftFilterLabel->setPixmap(QPixmap::fromImage(fftResultImageFiltred));
        fftFilterLabel->resize(fftFilterLabel->pixmap()->size());

        QVBoxLayout *fftFilterLayout = new QVBoxLayout(fftImageFiltredDialog);
        fftFilterLayout->addWidget(fftFilterLabel);
        fftImageFiltredDialog->setLayout(fftFilterLayout);
        fftImageFiltredDialog->show();
    }



//    QImage inverseFFTResultImage = performInverse2DFFT(filteredResult);


    // Отображаем результат обратного преобразования в новом диалоговом окне
    QDialog *inverseFFTImageDialog = new QDialog(this);
    QLabel *inverseFFTLabel = new QLabel(inverseFFTImageDialog);
    inverseFFTLabel->setPixmap(QPixmap::fromImage(inverseFFTResultImage));
    inverseFFTLabel->resize(inverseFFTLabel->pixmap()->size());

    QVBoxLayout *inverseFFTLayout = new QVBoxLayout(inverseFFTImageDialog);
    inverseFFTLayout->addWidget(inverseFFTLabel);
    inverseFFTImageDialog->setLayout(inverseFFTLayout);
    inverseFFTImageDialog->show();
}

void MainWindow::on_actionSave_triggered()
{
    if (inverseFFTResultImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No IFFT image loaded."));
        return;
    }
    QString filePath = QFileDialog::getSaveFileName(this, tr("Сохранить изображение"), "", tr("Файлы изображений (*.bmp)"));

    if (!filePath.isEmpty()) {
        // Сохранить изображение в выбранный файл BMP
        if (!inverseFFTResultImage.save(filePath, "BMP")) {
            // Обработка ошибки сохранения, если не удалось сохранить изображение
            qDebug() << "Ошибка сохранения файла";
        }
    }
}
