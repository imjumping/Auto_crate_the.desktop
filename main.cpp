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
#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>
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
        resize(850, 700);

        auto mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(15, 15, 15, 15);
        mainLayout->setSpacing(12);

        // Scroll area for all content
        auto scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        auto scrollContent = new QWidget();
        auto scrollLayout = new QVBoxLayout(scrollContent);
        scrollLayout->setContentsMargins(10, 10, 10, 10);
        scrollLayout->setSpacing(15);
        scrollArea->setWidget(scrollContent);
        mainLayout->addWidget(scrollArea);

        // Basic Information Group
        auto basicGroup = new QGroupBox("基本信息");
        auto basicLayout = new QVBoxLayout(basicGroup);
        
        // Name
        basicLayout->addWidget(new QLabel("应用名称 (Name): *"));
        nameEdit = new QLineEdit();
        nameEdit->setPlaceholderText("例如：MyApp");
        basicLayout->addWidget(nameEdit);
        
        // Generic Name
        basicLayout->addWidget(new QLabel("通用名称 (GenericName):"));
        genericNameEdit = new QLineEdit();
        genericNameEdit->setPlaceholderText("例如：文本编辑器");
        basicLayout->addWidget(genericNameEdit);
        
        // Comment
        basicLayout->addWidget(new QLabel("描述/注释 (Comment):"));
        commentEdit = new QLineEdit();
        commentEdit->setPlaceholderText("例如：一个简单的文本编辑器");
        basicLayout->addWidget(commentEdit);
        
        // Keywords
        basicLayout->addWidget(new QLabel("关键词 (Keywords):"));
        keywordsEdit = new QLineEdit();
        keywordsEdit->setPlaceholderText("用分号分隔，例如：text;editor;note");
        basicLayout->addWidget(keywordsEdit);
        
        scrollLayout->addWidget(basicGroup);

        // Execution Group
        auto execGroup = new QGroupBox("执行设置");
        auto execLayout = new QVBoxLayout(execGroup);
        
        // Exec Type
        execLayout->addWidget(new QLabel("执行方式:"));
        auto execTypeLayout = new QHBoxLayout();
        execFileRadio = new QRadioButton("选择可执行文件");
        execCmdRadio = new QRadioButton("手动输入命令");
        execFileRadio->setChecked(true);
        auto execGroupBtn = new QButtonGroup(this);
        execGroupBtn->addButton(execFileRadio);
        execGroupBtn->addButton(execCmdRadio);
        execTypeLayout->addWidget(execFileRadio);
        execTypeLayout->addWidget(execCmdRadio);
        execTypeLayout->addStretch();
        execLayout->addLayout(execTypeLayout);
        
        // Exec Path
        auto execPathLayout = new QHBoxLayout();
        execPathEdit = new QLineEdit();
        execPathEdit->setPlaceholderText("点击浏览或输入命令");
        browseExecBtn = new QPushButton("浏览...");
        execPathLayout->addWidget(execPathEdit, 4);
        execPathLayout->addWidget(browseExecBtn, 1);
        execLayout->addLayout(execPathLayout);
        
        // TryExec
        execLayout->addWidget(new QLabel("检查执行文件 (TryExec):"));
        auto tryExecLayout = new QHBoxLayout();
        tryExecEdit = new QLineEdit();
        tryExecEdit->setPlaceholderText("可选：用于检查程序是否安装");
        browseTryExecBtn = new QPushButton("浏览...");
        tryExecLayout->addWidget(tryExecEdit, 4);
        tryExecLayout->addWidget(browseTryExecBtn, 1);
        execLayout->addLayout(tryExecLayout);
        
        // Working Directory (Path)
        execLayout->addWidget(new QLabel("工作目录 (Path):"));
        auto pathLayout = new QHBoxLayout();
        pathEdit = new QLineEdit();
        pathEdit->setPlaceholderText("可选：程序启动时的工作目录");
        browsePathBtn = new QPushButton("浏览...");
        pathLayout->addWidget(pathEdit, 4);
        pathLayout->addWidget(browsePathBtn, 1);
        execLayout->addLayout(pathLayout);
        
        // Terminal and StartupNotify
        auto optionsLayout = new QHBoxLayout();
        terminalCheck = new QCheckBox("在终端中运行 (Terminal=true)");
        startupNotifyCheck = new QCheckBox("启动通知 (StartupNotify=true)");
        startupNotifyCheck->setChecked(true);
        optionsLayout->addWidget(terminalCheck);
        optionsLayout->addWidget(startupNotifyCheck);
        optionsLayout->addStretch();
        execLayout->addLayout(optionsLayout);
        
        scrollLayout->addWidget(execGroup);

        // Display and Visibility Group
        auto displayGroup = new QGroupBox("显示和可见性");
        auto displayLayout = new QVBoxLayout(displayGroup);
        
        // Icon
        displayLayout->addWidget(new QLabel("图标文件 (Icon):"));
        auto iconLayout = new QHBoxLayout();
        iconEdit = new QLineEdit();
        iconEdit->setPlaceholderText("可选：PNG/ICO/SVG 等，或图标名称如 'firefox'");
        browseIconBtn = new QPushButton("浏览图标...");
        iconLayout->addWidget(iconEdit, 4);
        iconLayout->addWidget(browseIconBtn, 1);
        displayLayout->addLayout(iconLayout);
        
        // Hidden and NoDisplay
        auto visibilityLayout = new QHBoxLayout();
        hiddenCheck = new QCheckBox("隐藏条目 (Hidden=true)");
        noDisplayCheck = new QCheckBox("不在菜单中显示 (NoDisplay=true)");
        visibilityLayout->addWidget(hiddenCheck);
        visibilityLayout->addWidget(noDisplayCheck);
        visibilityLayout->addStretch();
        displayLayout->addLayout(visibilityLayout);
        
        scrollLayout->addWidget(displayGroup);

        // Categories and MIME Types Group
        auto categoriesGroup = new QGroupBox("分类和 MIME 类型");
        auto categoriesLayout = new QVBoxLayout(categoriesGroup);
        
        // Categories
        categoriesLayout->addWidget(new QLabel("应用分类 (Categories):"));
        categoriesLayout->addWidget(new QLabel("按住 Ctrl 可选择多个分类"));
        categoriesCombo = new QComboBox();
        categoriesCombo->addItems({
            "Audio", "Video", "AudioVideo", "Development", "Education", 
            "Game", "Graphics", "Network", "Office", "Science", 
            "Settings", "System", "Utility", "Accessories", "Other"
        });
        categoriesCombo->setEditable(true);
        categoriesCombo->setInsertPolicy(QComboBox::NoInsert);
        categoriesLayout->addWidget(categoriesCombo);
        
        // MIME Types
        categoriesLayout->addWidget(new QLabel("MIME 类型 (MimeType):"));
        mimeTypeEdit = new QLineEdit();
        mimeTypeEdit->setPlaceholderText("可选：用分号分隔，例如：text/plain;text/html");
        categoriesLayout->addWidget(mimeTypeEdit);
        
        scrollLayout->addWidget(categoriesGroup);

        // Advanced Options Group
        auto advancedGroup = new QGroupBox("高级选项");
        auto advancedLayout = new QVBoxLayout(advancedGroup);
        
        // Version
        advancedLayout->addWidget(new QLabel("规范版本 (Version):"));
        versionEdit = new QLineEdit("1.0");
        versionEdit->setPlaceholderText("默认：1.0");
        advancedLayout->addWidget(versionEdit);
        
        // Desktop Environments
        advancedLayout->addWidget(new QLabel("仅在以下桌面环境中显示 (OnlyShowIn):"));
        onlyShowInEdit = new QLineEdit();
        onlyShowInEdit->setPlaceholderText("可选：用分号分隔，例如：GNOME;KDE");
        advancedLayout->addWidget(onlyShowInEdit);
        
        advancedLayout->addWidget(new QLabel("不在以下桌面环境中显示 (NotShowIn):"));
        notShowInEdit = new QLineEdit();
        notShowInEdit->setPlaceholderText("可选：用分号分隔，例如：Unity;XFCE");
        advancedLayout->addWidget(notShowInEdit);
        
        scrollLayout->addWidget(advancedGroup);

        // File Comments Group
        auto commentsGroup = new QGroupBox("文件注释");
        auto commentsLayout = new QVBoxLayout(commentsGroup);
        
        fileCommentsEdit = new QTextEdit();
        fileCommentsEdit->setPlaceholderText("# 在这里添加文件注释\n# 每行以 # 开头");
        fileCommentsEdit->setFont(QFont("Courier", 10));
        fileCommentsEdit->setMaximumHeight(80);
        commentsLayout->addWidget(fileCommentsEdit);
        
        scrollLayout->addWidget(commentsGroup);

        // Location Group
        auto locationGroup = new QGroupBox("保存位置");
        auto locationLayout = new QVBoxLayout(locationGroup);
        
        // Location options with both desktop paths
        locationLayout->addWidget(new QLabel("保存位置:"));
        
        locationDesktopRadio = new QRadioButton("保存到桌面 (~/桌面/)");
        locationDesktopEnRadio = new QRadioButton("保存到桌面 (~/Desktop/)");
        locationAppsRadio = new QRadioButton("保存到应用菜单 (~/.local/share/applications/)");
        locationCustomRadio = new QRadioButton("自定义位置:");
        
        locationDesktopRadio->setChecked(true);
        
        auto locationGroupBtn = new QButtonGroup(this);
        locationGroupBtn->addButton(locationDesktopRadio);
        locationGroupBtn->addButton(locationDesktopEnRadio);
        locationGroupBtn->addButton(locationAppsRadio);
        locationGroupBtn->addButton(locationCustomRadio);
        
        locationLayout->addWidget(locationDesktopRadio);
        locationLayout->addWidget(locationDesktopEnRadio);
        locationLayout->addWidget(locationAppsRadio);
        
        // Custom path
        auto customPathLayout = new QHBoxLayout();
        customPathEdit = new QLineEdit();
        customPathEdit->setPlaceholderText("输入自定义保存路径");
        browseCustomPathBtn = new QPushButton("浏览...");
        customPathLayout->addWidget(customPathEdit, 4);
        customPathLayout->addWidget(browseCustomPathBtn, 1);
        locationLayout->addLayout(customPathLayout);
        locationLayout->addWidget(locationCustomRadio);
        
        auto locationTipLabel = new QLabel(
            "💡 桌面：直接可见图标\n应用菜单：在启动器/应用程序列表中显示（需刷新）\n自定义：指定任意目录"
        );
        locationTipLabel->setStyleSheet("color: #666; font-size: 9pt;");
        locationLayout->addWidget(locationTipLabel);
        
        scrollLayout->addWidget(locationGroup);

        // Generate Button
        auto buttonLayout = new QHBoxLayout();
        generateBtn = new QPushButton("生成 .desktop 文件");
        generateBtn->setStyleSheet("font-weight: bold; padding: 10px; font-size: 11pt;");
        generateBtn->setMinimumHeight(40);
        buttonLayout->addStretch();
        buttonLayout->addWidget(generateBtn);
        buttonLayout->addStretch();
        
        auto helpBtn = new QPushButton("?");
        helpBtn->setToolTip("关于 .desktop 文件格式");
        helpBtn->setMaximumWidth(30);
        buttonLayout->addWidget(helpBtn);
        
        scrollLayout->addLayout(buttonLayout);
        scrollLayout->addStretch();

        // Connections
        connect(browseExecBtn, &QPushButton::clicked, this, &DesktopLauncherApp::browseExec);
        connect(browseTryExecBtn, &QPushButton::clicked, this, &DesktopLauncherApp::browseTryExec);
        connect(browsePathBtn, &QPushButton::clicked, this, &DesktopLauncherApp::browsePath);
        connect(browseIconBtn, &QPushButton::clicked, this, &DesktopLauncherApp::browseIcon);
        connect(browseCustomPathBtn, &QPushButton::clicked, this, &DesktopLauncherApp::browseCustomPath);
        connect(helpBtn, &QPushButton::clicked, this, &DesktopLauncherApp::showHelp);
        connect(execFileRadio, &QRadioButton::toggled, this, &DesktopLauncherApp::toggleExecInput);
        connect(generateBtn, &QPushButton::clicked, this, &DesktopLauncherApp::onGenerate);
        connect(locationCustomRadio, &QRadioButton::toggled, this, &DesktopLauncherApp::toggleCustomPath);
        
        // Initialize UI states
        toggleExecInput(true);
        toggleCustomPath(false);
        customPathEdit->setEnabled(false);
        browseCustomPathBtn->setEnabled(false);
    }

private slots:
    void browseExec() {
        QString path = QFileDialog::getOpenFileName(
            this,
            "选择可执行文件",
            QDir::homePath(),
            "所有文件 (*);;可执行文件 (*.exe *.bin *.sh *.out);;Shell 脚本 (*.sh);;Python 脚本 (*.py);;应用程序 (*.app)"
        );
        if (!path.isEmpty()) {
            execPathEdit->setText(path);
            // Auto-fill TryExec if empty
            if (tryExecEdit->text().isEmpty()) {
                tryExecEdit->setText(path);
            }
        }
    }

    void browseTryExec() {
        QString path = QFileDialog::getOpenFileName(
            this,
            "选择检查执行文件",
            QDir::homePath(),
            "所有文件 (*);;可执行文件 (*.exe *.bin *.sh *.out)"
        );
        if (!path.isEmpty()) {
            tryExecEdit->setText(path);
        }
    }

    void browsePath() {
        QString path = QFileDialog::getExistingDirectory(
            this,
            "选择工作目录",
            QDir::homePath(),
            QFileDialog::ShowDirsOnly
        );
        if (!path.isEmpty()) {
            pathEdit->setText(path);
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
            "所有文件 (*.*)"
        );
        if (!path.isEmpty()) {
            iconEdit->setText(path);
        }
    }

    void browseCustomPath() {
        QString path = QFileDialog::getExistingDirectory(
            this,
            "选择保存目录",
            QDir::homePath(),
            QFileDialog::ShowDirsOnly
        );
        if (!path.isEmpty()) {
            customPathEdit->setText(path);
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

    void toggleCustomPath(bool enabled) {
        customPathEdit->setEnabled(enabled);
        browseCustomPathBtn->setEnabled(enabled);
    }

    void showHelp() {
        QDialog dialog(this);
        dialog.setWindowTitle(".desktop 文件格式说明");
        dialog.setFixedSize(650, 450);
        
        auto layout = new QVBoxLayout(&dialog);
        
        auto textEdit = new QTextEdit();
        textEdit->setReadOnly(true);
        textEdit->setFont(QFont("Courier", 10));
        
        QString helpText = R"(
.desktop 文件基于 Freedesktop.org 规范，支持以下主要字段：

必需字段：
- Name: 应用名称
- Exec: 执行命令
- Type: 类型（通常为 Application）

常用字段：
- Comment: 描述信息
- Icon: 图标路径或名称
- Terminal: 是否在终端中运行 (true/false)
- Categories: 应用分类
- StartupNotify: 是否使用启动通知 (true/false)
- Path: 工作目录
- TryExec: 检查程序是否存在的路径
- Hidden/NoDisplay: 隐藏或不在菜单中显示
- Keywords: 搜索关键词
- MimeType: 支持的 MIME 类型
- Version: 规范版本
- OnlyShowIn/NotShowIn: 桌面环境过滤

字段类型：
- string: 普通字符串
- localestring: 可本地化的字符串 (Name[zh_CN])
- boolean: true/false
- string(s): 多个值用分号分隔 (Categories=Utility;Development;)

更多详情请参考：
https://specifications.freedesktop.org/desktop-entry-spec/latest/
)";
        
        textEdit->setText(helpText);
        layout->addWidget(textEdit);
        
        auto closeBtn = new QPushButton("关闭");
        connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
        layout->addWidget(closeBtn);
        
        dialog.exec();
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
            QFileInfo fi(execValue);
            if (fi.exists()) {
                execValue = fi.absoluteFilePath();
            }
        } else {
            if (execValue.isEmpty()) {
                QMessageBox::critical(this, "错误", "请输入命令！");
                return;
            }
        }

        QString tryExecValue = tryExecEdit->text().trimmed();
        if (!tryExecValue.isEmpty()) {
            QFileInfo fi(tryExecValue);
            if (fi.exists()) {
                tryExecValue = fi.absoluteFilePath();
            }
        }

        QString pathValue = pathEdit->text().trimmed();
        if (!pathValue.isEmpty()) {
            QFileInfo fi(pathValue);
            if (fi.exists() && fi.isDir()) {
                pathValue = fi.absoluteFilePath();
            } else {
                pathValue.clear();
            }
        }

        QString iconValue = iconEdit->text().trimmed();
        // 不再检查图标是否存在，也不显示警告

        QString targetDir;
        bool isCustomLocation = locationCustomRadio->isChecked();

        if (isCustomLocation) {
            targetDir = customPathEdit->text().trimmed();
            if (targetDir.isEmpty()) {
                QMessageBox::critical(this, "错误", "请指定自定义保存路径！");
                return;
            }
            if (!QDir(targetDir).exists()) {
                if (!QDir().mkpath(targetDir)) {
                    QMessageBox::critical(this, "错误", "无法创建目录: " + targetDir);
                    return;
                }
            }
        } else if (locationDesktopRadio->isChecked()) {
            targetDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            if (targetDir.isEmpty() || !QDir(targetDir).exists()) {
                targetDir = QDir::homePath() + "/桌面";
                if (!QDir(targetDir).exists()) {
                    if (!QDir().mkpath(targetDir)) {
                        QMessageBox::critical(this, "错误", "无法创建桌面目录！");
                        return;
                    }
                }
            }
        } else if (locationDesktopEnRadio->isChecked()) {
            targetDir = QDir::homePath() + "/Desktop";
            if (!QDir(targetDir).exists()) {
                if (!QDir().mkpath(targetDir)) {
                    QMessageBox::critical(this, "错误", "无法创建 Desktop 目录！");
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

        // Build .desktop content with file comments
        QString content = "";
        
        // Add file comments if any
        QString comments = fileCommentsEdit->toPlainText().trimmed();
        if (!comments.isEmpty()) {
            content += comments + "\n\n";
        }
        
        content += QString(
            "[Desktop Entry]\n"
            "Version=%1\n"
            "Type=Application\n"
            "Name=%2\n"
        ).arg(versionEdit->text().trimmed().isEmpty() ? "1.0" : versionEdit->text().trimmed())
         .arg(name);

        // Add optional fields only if they have values
        if (!genericNameEdit->text().isEmpty()) {
            content += QString("GenericName=%1\n").arg(genericNameEdit->text().trimmed());
        }
        
        if (!commentEdit->text().isEmpty()) {
            content += QString("Comment=%1\n").arg(commentEdit->text().trimmed());
        }
        
        content += QString("Exec=%1\n").arg(execValue);
        
        if (!tryExecValue.isEmpty()) {
            content += QString("TryExec=%1\n").arg(tryExecValue);
        }
        
        if (!iconValue.isEmpty()) {
            content += QString("Icon=%1\n").arg(iconValue);
        }
        
        if (!pathValue.isEmpty()) {
            content += QString("Path=%1\n").arg(pathValue);
        }
        
        if (!categoriesCombo->currentText().isEmpty()) {
            QString categories = categoriesCombo->currentText().trimmed();
            if (!categories.endsWith(";")) {
                categories += ";";
            }
            content += QString("Categories=%1\n").arg(categories);
        }
        
        if (!mimeTypeEdit->text().isEmpty()) {
            QString mimeTypes = mimeTypeEdit->text().trimmed();
            if (!mimeTypes.endsWith(";")) {
                mimeTypes += ";";
            }
            content += QString("MimeType=%1\n").arg(mimeTypes);
        }
        
        content += QString("Terminal=%1\n").arg(terminalCheck->isChecked() ? "true" : "false");
        content += QString("StartupNotify=%1\n").arg(startupNotifyCheck->isChecked() ? "true" : "false");
        
        if (hiddenCheck->isChecked()) {
            content += "Hidden=true\n";
        }
        
        if (noDisplayCheck->isChecked()) {
            content += "NoDisplay=true\n";
        }
        
        if (!keywordsEdit->text().isEmpty()) {
            QString keywords = keywordsEdit->text().trimmed();
            if (!keywords.endsWith(";")) {
                keywords += ";";
            }
            content += QString("Keywords=%1\n").arg(keywords);
        }
        
        if (!onlyShowInEdit->text().isEmpty()) {
            QString onlyShowIn = onlyShowInEdit->text().trimmed();
            if (!onlyShowIn.endsWith(";")) {
                onlyShowIn += ";";
            }
            content += QString("OnlyShowIn=%1\n").arg(onlyShowIn);
        }
        
        if (!notShowInEdit->text().isEmpty()) {
            QString notShowIn = notShowInEdit->text().trimmed();
            if (!notShowIn.endsWith(";")) {
                notShowIn += ";";
            }
            content += QString("NotShowIn=%1\n").arg(notShowIn);
        }

        QFile file(filepath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误", "无法写入文件：" + file.errorString());
            return;
        }
        file.write(content.toUtf8());
        file.close();

        // Set executable permission (chmod +x)
        ::chmod(filepath.toLocal8Bit().constData(), 0755);

        QString successMessage;
        if (isCustomLocation) {
            successMessage = QString(".desktop 文件已生成至：\n%1").arg(filepath);
        } else if (locationDesktopRadio->isChecked() || locationDesktopEnRadio->isChecked()) {
            successMessage = QString(".desktop 文件已生成至桌面：\n%1\n\n双击即可运行。").arg(filepath);
        } else {
            successMessage = QString(".desktop 文件已生成至应用菜单：\n%1").arg(filepath);
        }

        // Show success dialog with refresh instructions for app menu
        if (!locationAppsRadio->isChecked() || isCustomLocation) {
            QMessageBox::information(this, "成功", successMessage);
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
            dialog.setFixedSize(680, 340);

            auto layout = new QVBoxLayout(&dialog);

            layout->addWidget(new QLabel(successMessage));
            auto tipLabel = new QLabel(tipText);
            tipLabel->setWordWrap(true);
            tipLabel->setStyleSheet("color: #555; font-size: 9pt;");
            layout->addWidget(tipLabel);

            auto cmdEdit = new QLineEdit(refreshCmd);
            cmdEdit->setReadOnly(true);
            cmdEdit->setFont(QFont("Courier", 10));
            layout->addWidget(cmdEdit);

            auto copyBtnLayout = new QHBoxLayout();
            auto copyBtn = new QPushButton("📋 复制命令");
            auto testBtn = new QPushButton("⚙️ 验证 .desktop 文件");
            copyBtnLayout->addWidget(copyBtn);
            copyBtnLayout->addWidget(testBtn);
            copyBtnLayout->addStretch();
            layout->addLayout(copyBtnLayout);

            connect(copyBtn, &QPushButton::clicked, [&]() {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(refreshCmd);
                QMessageBox::information(&dialog, "复制成功", "命令已复制到剪贴板！");
            });

            connect(testBtn, &QPushButton::clicked, [&]() {
                QString validateCmd = "desktop-file-validate \"" + filepath + "\"";
                QProcess process;
                process.start("/bin/sh", QStringList() << "-c" << validateCmd);
                if (process.waitForFinished(3000)) {
                    QString output = process.readAllStandardOutput();
                    QString error = process.readAllStandardError();
                    QString result = output.isEmpty() ? "✅ 验证成功，文件格式正确！" : 
                                    "⚠️ 验证警告：\n" + output;
                    if (!error.isEmpty()) {
                        result += "\n❌ 验证错误：\n" + error;
                    }
                    QMessageBox::information(&dialog, "验证结果", result);
                } else {
                    QMessageBox::warning(&dialog, "验证失败", 
                        "无法运行 desktop-file-validate 命令。\n"
                        "请先安装 desktop-file-utils 包。");
                }
            });

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
    QLineEdit *genericNameEdit;
    QLineEdit *commentEdit;
    QLineEdit *keywordsEdit;
    QRadioButton *execFileRadio, *execCmdRadio;
    QLineEdit *execPathEdit;
    QPushButton *browseExecBtn;
    QLineEdit *tryExecEdit;
    QPushButton *browseTryExecBtn;
    QLineEdit *pathEdit;
    QPushButton *browsePathBtn;
    QCheckBox *terminalCheck;
    QCheckBox *startupNotifyCheck;
    QLineEdit *iconEdit;
    QPushButton *browseIconBtn;
    QCheckBox *hiddenCheck;
    QCheckBox *noDisplayCheck;
    QComboBox *categoriesCombo;
    QLineEdit *mimeTypeEdit;
    QLineEdit *versionEdit;
    QLineEdit *onlyShowInEdit;
    QLineEdit *notShowInEdit;
    QTextEdit *fileCommentsEdit;
    QRadioButton *locationDesktopRadio;
    QRadioButton *locationDesktopEnRadio;
    QRadioButton *locationAppsRadio;
    QRadioButton *locationCustomRadio;
    QLineEdit *customPathEdit;
    QPushButton *browseCustomPathBtn;
    QPushButton *generateBtn;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application name for QStandardPaths
    app.setApplicationName("DesktopLauncher");
    app.setOrganizationName("Local");
    app.setStyle("Fusion");

    DesktopLauncherApp window;
    window.show();

    return app.exec();
}