/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "languagechooser.h"
#include "mainwindow.h"

#include <QCoreApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QTranslator>

LanguageChooser::LanguageChooser(const QString &defaultLang, QWidget *parent)
    : QDialog(parent, Qt::WindowStaysOnTopHint)
{
    groupBox = new QGroupBox("Languages");

    QGridLayout *groupBoxLayout = new QGridLayout;

    const QStringList qmFiles = findQmFiles();
    for (int i = 0; i < qmFiles.size(); ++i) {
        const QString &qmlFile = qmFiles.at(i);
        QCheckBox *checkBox = new QCheckBox(languageName(qmlFile));
        // ??????checkbox->qmFile???map????????????????????????
        qmFileForCheckBoxMap.insert(checkBox, qmlFile);
        connect(checkBox, &QCheckBox::toggled,
                this, &LanguageChooser::checkBoxToggled);

        // ????????????????????????????????????????????????????????????CheckBox
        if (languageMatch(defaultLang, qmlFile))
            checkBox->setCheckState(Qt::Checked);
        groupBoxLayout->addWidget(checkBox, i / 2, i % 2);
    }
    groupBox->setLayout(groupBoxLayout);

    buttonBox = new QDialogButtonBox;
    showAllButton = buttonBox->addButton("Show All",
                                         QDialogButtonBox::ActionRole);
    hideAllButton = buttonBox->addButton("Hide All",
                                         QDialogButtonBox::ActionRole);

    connect(showAllButton, &QAbstractButton::clicked, this, &LanguageChooser::showAll);
    connect(hideAllButton, &QAbstractButton::clicked, this, &LanguageChooser::hideAll);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBox);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle("I18N");
}

// defaultLang = "zh_CN"
// qmFile = "i18n_ar.qm"
bool LanguageChooser::languageMatch(const QString &lang, const QString &qmFile)
{
    //qmFile: i18n_xx.qm
    const QString prefix = "i18n_";
    const int langTokenLength = 2; /*FIXME: is checking two chars enough?*/
    return qmFile.midRef(qmFile.indexOf(prefix) + prefix.length(), langTokenLength) == lang.leftRef(langTokenLength);
}

bool LanguageChooser::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Close) {
        MainWindow *window = qobject_cast<MainWindow *>(object);
        if (window) {
            QCheckBox *checkBox = mainWindowForCheckBoxMap.key(window);
            if (checkBox)
                checkBox->setChecked(false);
        }
    }
    return QDialog::eventFilter(object, event);
}

void LanguageChooser::closeEvent(QCloseEvent * /* event */)
{
    QCoreApplication::quit();
}

// ??????CheckBox????????????????????????
void LanguageChooser::checkBoxToggled()
{
    // sender()??????????????????????????????????????????????????????checkbox?????????
    // ?????????????????????????????????CheckBox?????????????????????mainwindow???qmfile
    QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());

    // ????????????map????????????????????????????????????
    MainWindow *window = mainWindowForCheckBoxMap.value(checkBox);
    // ??????window????????????????????????????????????????????????if?????????
    if (!window) {
        QTranslator translator;
        // ??????qmfile??????
        translator.load(qmFileForCheckBoxMap.value(checkBox));
        // qApp ??????QCoreApplication??????????????????main???????????????
        qApp->installTranslator(&translator);

        window = new MainWindow;
        window->setPalette(colorForLanguage(checkBox->text()));

        window->installEventFilter(this);

        // mainWindow???????????????map???????????????????????????????????????
        mainWindowForCheckBoxMap.insert(checkBox, window);
    }
    window->setVisible(checkBox->isChecked());
}

// ???????????????????????????mainwindow???setvisible????????????
// ?????????????????????CheckBox????????????
void LanguageChooser::showAll()
{
    for (auto it = qmFileForCheckBoxMap.keyBegin(); it != qmFileForCheckBoxMap.keyEnd(); ++it)
        (*it)->setChecked(true);
}

void LanguageChooser::hideAll()
{
    for (auto it = qmFileForCheckBoxMap.keyBegin(); it != qmFileForCheckBoxMap.keyEnd(); ++it)
        (*it)->setChecked(false);
}

QStringList LanguageChooser::findQmFiles()
{
    QDir dir(":/translations");
    // ?????????QDir::Name?????????????????????
    QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files,
                                          QDir::Name);

    // ????????????????????????????????????stringlist????????????
    for (QString &fileName : fileNames)
        fileName = dir.filePath(fileName);
    return fileNames;
}

// ????????????????????????
QString LanguageChooser::languageName(const QString &qmFile)
{
    QTranslator translator;
    translator.load(qmFile);
    // ????????????mainwindow??????englist?????????????????????????????????????????????????????????
    return translator.translate("MainWindow", "English");
}

QColor LanguageChooser::colorForLanguage(const QString &language)
{
    uint hashValue = qHash(language);
    int red = 156 + (hashValue & 0x3F);
    int green = 156 + ((hashValue >> 6) & 0x3F);
    int blue = 156 + ((hashValue >> 12) & 0x3F);
    return QColor(red, green, blue);
}
