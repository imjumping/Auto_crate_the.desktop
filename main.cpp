#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QTextEdit>
#include <QDialog>
#include <QFormLayout>
#include <QSpacerItem>
#include <QStandardPaths>
#include <QSysInfo>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

class DesktopLauncherApp : public QWidget {
    Q_OBJECT

public:
    DesktopLauncherApp(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("创建 .desktop 启动器");
        resize(650, 580);

        auto mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);

        // Name
        mainLayout->addWidget(new QLabel("应用名称 (Name):"));
        nameEdit = new QLineEdit();
        nameEdit->setPlaceholderText("例如：MyApp");
        mainLayout->addWidget(nameEdit);
        mainLayout->addSpacing(10);

        // Exec Type
        mainLayout->addWidget(new QLabel("执行方式:"));
        auto execTypeLayout = new QHBoxLayout();
        execFileRadio = new QRadioButton("选择可执行文件");
        execCmdRadio = new QRadioButton("手动输入命令");
        execFileRadio->setChecked(true);
        auto execGroup = new QButtonGroup(this);
        execGroup->addButton(execFileRadio);
        execGroup->addButton(execCmdRadio);
        execTypeLayout->addWidget(execFileRadio);
        execTypeLayout->addWidget(execCmdRadio);
        execTypeLayout->addStretch();
        mainLayout->addLayout(execTypeLayout);
        mainLayout->addSpacing(10);

        // Exec Path
        auto execPathLayout = new QHBoxLayout();
        execPathEdit = new QLineEdit();
        execPathEdit->setPlaceholderText("点击浏览或输入命令");
        browseExecBtn = new QPushButton("浏览...");
        execPathLayout->addWidget(execPathEdit);
        execPathLayout->addWidget(browseExecBtn);
        mainLayout->addLayout(execPathLayout);
        mainLayout->addSpacing(15);

        // Icon
        mainLayout->addWidget(new QLabel("图标文件 (Icon):"));
        auto iconLayout = new QHBoxLayout();
        iconEdit = new QLineEdit();
        iconEdit->setPlaceholderText("可选：PNG/ICO/SVG 等");
        browseIconBtn = new QPushButton("浏览图标...");
        iconLayout->addWidget(iconEdit);
        iconLayout->addWidget(browseIconBtn);
        mainLayout->addLayout(iconLayout);
        mainLayout->addSpacing(15);

        // Terminal
        terminalCheck = new QCheckBox("在终端中运行 (Terminal=true)");
        mainLayout->addWidget(terminalCheck);
        mainLayout->addSpacing(15);

        // Location
        mainLayout->addWidget(new QLabel("保存位置:"));
        locationDesktopRadio = new QRadioButton("保存到桌面 (~/桌面/)");
        locationAppsRadio = new QRadioButton("保存到应用菜单 (~/.local/share/applications/)");
        locationDesktopRadio->setChecked(true);
        auto locationGroup = new QButtonGroup(this);
        locationGroup->addButton(locationDesktopRadio);
        locationGroup->addButton(locationAppsRadio);
        mainLayout->addWidget(locationDesktopRadio);
        mainLayout->addWidget(locationAppsRadio);

        auto tipLabel = new QLabel(
            "💡 桌面：直接可见图标\n应用菜单：在启动器/应用程序列表中显示（需刷新）"
        );
        tipLabel->setStyleSheet("color: #666;");
        tipLabel->setFont(QFont("Arial", 9));
        mainLayout->addWidget(tipLabel);
        mainLayout->addSpacing(15);

        // Generate Button
        generateBtn = new QPushButton("生成 .desktop 文件");
        generateBtn->setStyleSheet("font-weight: bold; padding: 8px;");
        mainLayout->addWidget(generateBtn);
        mainLayout->addStretch();

        // Connections
        connect(browseExecBtn, &QPushButton::clicked, this, &DesktopLauncherApp::browseExec);
        connect(browseIconBtn, &QPushButton::clicked, this, &DesktopLauncherApp::browseIcon);
        connect(execFileRadio, &QRadioButton::toggled, this, &DesktopLauncherApp::toggleExecInput);
        connect(generateBtn, &QPushButton::clicked, this, &DesktopLauncherApp::onGenerate);

        toggleExecInput(true);
    }

private slots:
    void browseExec() {
        QString path = QFileDialog::getOpenFileName(
            this,
            "选择可执行文件",
            QDir::homePath(),
            "所有文件 (*);;可执行文件 (*.exe *.bin *.sh *.out);;Shell 脚本 (*.sh);;Python 脚本 (*.py)"
        );
        if (!path.isEmpty()) {
            execPathEdit->setText(path);
        }
    }

    void browseIcon() {
        QString path = QFileDialog::getOpenFileName(
            this,
            "选择图标文件",
            QDir::homePath(),
            "图标文件 (*.png *.ico *.svg *.jpg *.jpeg *.bmp);;"
            "PNG 图像 (*.png);;"
            "ICO 图标 (*.ico);;"
            "SVG 图像 (*.svg);;"
            "JPEG 图像 (*.jpg *.jpeg);;"
            "BMP 图像 (*.bmp);;"
            "所有文件 (*.*)"
        );
        if (!path.isEmpty()) {
            iconEdit->setText(path);
        }
    }

    void toggleExecInput(bool isFileMode) {
        browseExecBtn->setEnabled(isFileMode);
        if (isFileMode) {
            execPathEdit->setPlaceholderText("点击浏览选择可执行文件");
        } else {
            execPathEdit->setPlaceholderText("输入完整命令，如：/usr/bin/myapp --option");
        }
    }

    void onGenerate() {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::critical(this, "错误", "应用名称不能为空！");
            return;
        }

        QString execValue = execPathEdit->text().trimmed();
        if (execFileRadio->isChecked()) {
            if (execValue.isEmpty()) {
                QMessageBox::critical(this, "错误", "请选择或输入可执行路径！");
                return;
            }
            // Convert to absolute path
            QFileInfo fi(execValue);
            execValue = fi.absoluteFilePath();
        } else {
            if (execValue.isEmpty()) {
                QMessageBox::critical(this, "错误", "请输入命令！");
                return;
            }
        }

        QString iconValue = iconEdit->text().trimmed();
        if (!iconValue.isEmpty() && !QFileInfo::exists(iconValue)) {
            QMessageBox::warning(this, "警告", "图标文件不存在，将使用默认图标（仍会生成 .desktop 文件）");
        }

        bool terminal = terminalCheck->isChecked();
        QString targetDir;
        bool isDesktop = locationDesktopRadio->isChecked();

        if (isDesktop) {
            targetDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            if (targetDir.isEmpty()) {
                targetDir = QDir::homePath() + "/桌面";
            }
            if (!QDir(targetDir).exists()) {
                if (!QDir().mkpath(targetDir)) {
                    QMessageBox::critical(this, "错误", "无法创建桌面目录！");
                    return;
                }
            }
        } else {
            targetDir = QDir::homePath() + "/.local/share/applications";
            if (!QDir(targetDir).exists()) {
                if (!QDir().mkpath(targetDir)) {
                    QMessageBox::critical(this, "错误", "无法创建应用目录！");
                    return;
                }
            }
        }

        QString filename = name + ".desktop";
        QString filepath = targetDir + "/" + filename;

        // Build .desktop content
        QString content = QString(
            "[Desktop Entry]\n"
            "Version=1.0\n"
            "Type=Application\n"
            "Name=%1\n"
            "Exec=%2\n"
            "Icon=%3\n"
            "Terminal=%4\n"
            "Categories=Utility;\n"
        ).arg(name)
         .arg(execValue)
         .arg(iconValue)
         .arg(terminal ? "true" : "false");

        QFile file(filepath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误", "无法写入文件：" + file.errorString());
            return;
        }
        file.write(content.toUtf8());
        file.close();

        // Set executable permission (chmod +x)
        ::chmod(filepath.toLocal8Bit().constData(), 0755);

        if (isDesktop) {
            QMessageBox::information(this, "成功",
                QString(".desktop 文件已生成至桌面：\n%1\n\n双击即可运行。").arg(filepath)
            );
            qApp->quit();
        } else {
            QString refreshCmd = "update-desktop-database ~/.local/share/applications";
            bool hasUpdateCmd = QProcess::execute("which", QStringList() << "update-desktop-database") == 0;

            QString tipText;
            if (hasUpdateCmd) {
                tipText = "✅ 检测到 update-desktop-database 已安装，运行以下命令可立即刷新菜单：";
            } else {
                tipText = "⚠️ 未检测到 update-desktop-database，请先安装：\n"
                          "  Ubuntu/Debian:   sudo apt install desktop-file-utils\n"
                          "  Fedora/RHEL:     sudo dnf install desktop-file-utils\n"
                          "  Arch/Manjaro:    sudo pacman -S desktop-file-utils\n"
                          "安装后运行以下命令刷新菜单：";
            }

            QDialog dialog(this);
            dialog.setWindowTitle("操作提示");
            dialog.setModal(true);
            dialog.setFixedSize(650, 320);

            auto layout = new QVBoxLayout(&dialog);

            layout->addWidget(new QLabel(QString(".desktop 文件已生成至应用菜单：\n%1").arg(filepath)));
            auto tipLabel = new QLabel(tipText);
            tipLabel->setWordWrap(true);
            tipLabel->setStyleSheet("color: #555; font-size: 9pt;");
            layout->addWidget(tipLabel);

            auto cmdEdit = new QLineEdit(refreshCmd);
            cmdEdit->setReadOnly(true);
            cmdEdit->setFont(QFont("Courier", 10));
            layout->addWidget(cmdEdit);

            auto copyBtn = new QPushButton("📋 复制命令");
            connect(copyBtn, &QPushButton::clicked, [&]() {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(refreshCmd);
                QMessageBox::information(&dialog, "复制成功", "命令已复制到剪贴板！");
            });
            layout->addWidget(copyBtn);

            auto closeBtn = new QPushButton("关闭");
            connect(closeBtn, &QPushButton::clicked, [&]() {
                dialog.accept();
                qApp->quit();
            });
            layout->addWidget(closeBtn);

            dialog.exec();
        }
    }

private:
    QLineEdit *nameEdit;
    QRadioButton *execFileRadio, *execCmdRadio;
    QLineEdit *execPathEdit;
    QPushButton *browseExecBtn;
    QLineEdit *iconEdit;
    QPushButton *browseIconBtn;
    QCheckBox *terminalCheck;
    QRadioButton *locationDesktopRadio, *locationAppsRadio;
    QPushButton *generateBtn;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application name for QStandardPaths
    app.setApplicationName("DesktopLauncher");
    app.setOrganizationName("Local");

    DesktopLauncherApp window;
    window.show();

    return app.exec();
}
