#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDebug>
#include <QScreen>
#include <QSqlDatabase>
#include <QScrollArea>
#include <QFrame>
#include <QMenu>
#include <QShortcut>
#include <QTimer>
static bool login = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    originalStyleSheet = this->styleSheet();
    ui->lineEdit->setFocus();  // Set focus initially

    connect(ui->lineEdit, &QLineEdit::returnPressed, ui->lineEdit_2, QOverload<>::of(&QLineEdit::setFocus));
    connect(ui->lineEdit_2, &QLineEdit::returnPressed, this, &MainWindow::on_addbus_clicked);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearCentralWidget()
{
    QLayout *layout = ui->centralwidget->layout();
    if (layout) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
            if (QWidget *widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
        delete layout;
    }
    ui->centralwidget->setLayout(nullptr);
}

void MainWindow::showWelcomeUI(const QString &username) {
    QVBoxLayout *layout = new QVBoxLayout;
    ui->centralwidget->setLayout(layout);

    QLabel *userLabel = new QLabel(username, this);
    userLabel->setStyleSheet("color: white; background-color: #005E99; padding: 5px; border-radius: 10px;");
    userLabel->setFixedSize(150, 30);

    int margin = 10;
    int x = this->width() - userLabel->width() - margin;
    int y = margin;
    userLabel->move(x, y);
    userLabel->show();

    QPushButton *loginButtonUi = ui->centralwidget->findChild<QPushButton*>("pushButton_2");
    if (loginButtonUi) loginButtonUi->deleteLater();

    QPushButton *signupButtonUi = ui->centralwidget->findChild<QPushButton*>("pushButton_3");
    if (signupButtonUi) signupButtonUi->deleteLater();
}

void MainWindow::checkBus() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "addbus_conn");
    db.setDatabaseName("D:/project/database/addbus.db");

    if (!db.open()) {
        qDebug() << "Database connection error";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT [From], [To], [Time], [BusId] FROM Bus WHERE Username = :u");
    query.bindValue(":u", currentUsername);

    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return;
    }

    busList.clear();

    while (query.next()) {
        BusInfo bus;
        bus.from = query.value("From").toString();
        bus.to = query.value("To").toString();
        bus.time = query.value("Time").toString();
        bus.busid=query.value("BusId").toString();
        busList.append(bus);
    }

    db.close();
    showOverlayScrollArea();
}

void MainWindow::showOverlayScrollArea() {
    // Delete previous scroll area if it exists
    if (scrollArea) {
        scrollArea->deleteLater();
        scrollArea = nullptr;
    }

    // Create a new scroll area
    scrollArea = new QScrollArea(this);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "  background-color: rgba(255, 255, 255, 230);"
        "  border-radius: 12px;"
        "}"
        );
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->raise();

    // Set dimensions and position
    int screenWidth = this->width();
    int screenHeight = this->height();
    int scrollWidth = screenWidth * 0.8;
    int scrollHeight = screenHeight * 0.2;
    int scrollX = screenWidth * 0.1;
    int scrollY = screenHeight * 0.8;

    scrollArea->setGeometry(scrollX, scrollY, scrollWidth, scrollHeight);

    // Content inside the scroll area
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(contentWidget);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    // Add "No buses found" label initially
    QLabel *noBusLabel = new QLabel("No buses found");
    noBusLabel->setAlignment(Qt::AlignCenter);
    noBusLabel->setStyleSheet("color: gray; font-size: 16px;");
    layout->addWidget(noBusLabel);

    bool busAdded = false;

    // Iterate through each BusInfo and populate rows
    for (const BusInfo &bus : busList) {
        busAdded = true;

        if (noBusLabel) {
            layout->removeWidget(noBusLabel);
            noBusLabel->deleteLater();
            noBusLabel = nullptr;
        }

        QHBoxLayout *rowLayout = new QHBoxLayout;

        QLabel *fromLabel = new QLabel("From:");
        fromLabel->setStyleSheet("font-weight: bold;");
        QLineEdit *fromEdit = new QLineEdit(bus.from);
        fromEdit->setReadOnly(true);
        fromEdit->setFixedWidth(100);
        fromEdit->setStyleSheet("background: #E6F0FA; border: 1px solid #3399FF; border-radius: 5px; padding: 5px;");

        QLabel *toLabel = new QLabel("To:");
        toLabel->setStyleSheet("font-weight: bold;");
        QLineEdit *toEdit = new QLineEdit(bus.to);
        toEdit->setReadOnly(true);
        toEdit->setFixedWidth(100);
        toEdit->setStyleSheet("background: #E6F0FA; border: 1px solid #3399FF; border-radius: 5px; padding: 5px;");

        QLabel *timeLabel = new QLabel("Time:");
        timeLabel->setStyleSheet("font-weight: bold;");
        QLineEdit *timeEdit = new QLineEdit(bus.time);
        timeEdit->setReadOnly(true);
        timeEdit->setFixedWidth(100);
        timeEdit->setStyleSheet("background: #E6F0FA; border: 1px solid #3399FF; border-radius: 5px; padding: 5px;");

        QLabel *idLabel = new QLabel("Number Plate:");
        idLabel->setStyleSheet("font-weight: bold;");
        QLineEdit *idEdit = new QLineEdit(bus.busid);
        idEdit->setReadOnly(true);
        idEdit->setFixedWidth(60);
        idEdit->setStyleSheet("background: #F6F6F6; border: 1px solid #CCCCCC; border-radius: 5px; padding: 5px;");

        QPushButton *optionsButton = new QPushButton("⋮");
        optionsButton->setFixedWidth(30);
        optionsButton->setStyleSheet("border: none; font-size: 20px;");

        QMenu *menu = new QMenu(optionsButton);
        QAction *deleteAction = new QAction("Delete");
        QAction *modifyAction = new QAction("Modify");
        menu->addAction(deleteAction);
        menu->addAction(modifyAction);
        optionsButton->setMenu(menu);

        // Delete bus from database
        connect(deleteAction, &QAction::triggered, this, [=]() {

            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("D:/project/database/addbus.db");

            if (!db.open()) {
                qDebug() << "Database open failed:" << db.lastError().text();
                return;
            }

            QSqlQuery deleteQuery(db);
            deleteQuery.prepare("DELETE FROM Bus WHERE BusId = :id AND Username = :u");
            deleteQuery.bindValue(":id", bus.busid);
            deleteQuery.bindValue(":u", currentUsername);

            if (!deleteQuery.exec()) {
                qDebug() << "Delete failed:" << deleteQuery.lastError().text();
            } else {
                qDebug() << "Deleted successfully, rows affected:" << deleteQuery.numRowsAffected();
            }

            db.close();

            checkBus(); // Refresh
        });




        // Modify bus
        connect(modifyAction, &QAction::triggered, this, [=]() {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("D:/project/database/addbus.db");

            if (!db.open()) {
                qDebug() << "Database open failed:" << db.lastError().text();
                return;
            }

            QSqlQuery deleteQuery(db);
            deleteQuery.prepare("DELETE FROM Bus WHERE BusId = :id AND Username = :u");
            deleteQuery.bindValue(":id", bus.busid);
            deleteQuery.bindValue(":u", currentUsername);

            if (!deleteQuery.exec()) {
                qDebug() << "Delete failed:" << deleteQuery.lastError().text();
            } else {
                qDebug() << "Deleted successfully, rows affected:" << deleteQuery.numRowsAffected();
            }
            db.close();
            on_addbus_clicked();
        });

        rowLayout->addWidget(fromLabel);
        rowLayout->addWidget(fromEdit);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(toLabel);
        rowLayout->addWidget(toEdit);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(timeLabel);
        rowLayout->addWidget(timeEdit);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(idLabel);
        rowLayout->addWidget(idEdit);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(optionsButton);

        layout->addLayout(rowLayout);
    }


    layout->addStretch();
    scrollArea->setWidget(contentWidget);
    scrollArea->show();
}

void MainWindow::on_pushButton_2_clicked()
{
    clearCentralWidget();

    QVBoxLayout *wrapperLayout = new QVBoxLayout;

    QFrame *formFrame = new QFrame;
    formFrame->setFixedSize(1200, 500);
    formFrame->setStyleSheet(
        "background-image: url('D:/project/admin_pannel/login_bg.png');"
        "background-repeat: no-repeat;"
        "background-position: center;"
        "border-radius: 20px;"
        );

    QGridLayout *gridLayout = new QGridLayout(formFrame);
    gridLayout->setContentsMargins(30, 80, 30, 80); // left, top, right, bottom
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(25);

    // === Widgets ===
    QLabel *lblUsername = new QLabel("Username:");
    QLabel *lblPassword = new QLabel("Password:");


    QString labelStyle = "color: white;";
    for (QLabel *label : {lblUsername, lblPassword}) {

        label->setStyleSheet(labelStyle);
        label->setFixedWidth(140);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    QLineEdit *editUsername = new QLineEdit;
    editUsername->setPlaceholderText("Enter your username");

    QLineEdit *editPassword = new QLineEdit;
    editPassword->setPlaceholderText("Enter password");
    editPassword->setEchoMode(QLineEdit::Password);

    QString lineEditStyle = R"(
        QLineEdit {
            background: rgba(255, 255, 255, 0.9);
            border: 1px solid #ccc;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 14px;
        })";
    for (QLineEdit *edit : {editUsername, editPassword}) {
        edit->setFixedWidth(320);
        edit->setStyleSheet(lineEditStyle);
    }

    QPushButton *loginbtn = new QPushButton("Login");
    QPushButton *signupbtn = new QPushButton("Signup");

    // === Add to Grid ===
    gridLayout->addWidget(lblUsername, 0, 0, Qt::AlignRight);
    gridLayout->addWidget(editUsername, 0, 1, Qt::AlignLeft);

    gridLayout->addWidget(lblPassword, 1, 0, Qt::AlignRight);
    gridLayout->addWidget(editPassword, 1, 1, Qt::AlignLeft);

    // === Button Styles ===
    QString buttonStyle = R"(
        QPushButton {
            background-color: #3c8dbc;
            color: white;
            padding: 10px 20px;
            font-size: 14px;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: #3272a1;
        })";
    loginbtn->setStyleSheet(buttonStyle);
    signupbtn->setStyleSheet(buttonStyle);
    loginbtn->setFixedWidth(120);
    signupbtn->setFixedWidth(120);

    // === Center Both Buttons Between Columns ===
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(loginbtn);
    btnLayout->addSpacing(20);
    btnLayout->addWidget(signupbtn);
    btnLayout->addStretch();

    QWidget *buttonWrapper = new QWidget;
    buttonWrapper->setLayout(btnLayout);
    gridLayout->addWidget(buttonWrapper, 2, 0, 1, 2); // span 2 columns

    // === Focus and Return Handling ===
    QTimer::singleShot(1, editUsername, SLOT(setFocus()));
    connect(editUsername, &QLineEdit::returnPressed, editPassword, QOverload<>::of(&QLineEdit::setFocus));
    connect(editPassword, &QLineEdit::returnPressed, loginbtn, &QPushButton::click);

    // === Layout Setup ===
    wrapperLayout->addStretch();
    wrapperLayout->addWidget(formFrame, 0, Qt::AlignCenter);
    wrapperLayout->addStretch();
    ui->centralwidget->setLayout(wrapperLayout);

    // === Login Logic ===
    connect(loginbtn, &QPushButton::clicked, this, [=]() {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "login_conn");
        db.setDatabaseName("D:/project/database/login_info.db");

        if (!db.open()) {
            QMessageBox::warning(this, "DB Error", db.lastError().text());
            return;
        }

        QSqlQuery query(db);
        query.prepare("SELECT * FROM login WHERE username = :u AND password = :p");
        query.bindValue(":u", editUsername->text());
        query.bindValue(":p", editPassword->text());

        if (query.exec() && query.next()) {
            login = true;
            currentUsername = editUsername->text();
            QMessageBox::information(this, "Login", "Success");
            clearCentralWidget();
            showWelcomeUI(currentUsername);
            checkBus();
        } else {
            QMessageBox::warning(this, "Login Failed", "Invalid credentials");
        }
        db.close();
    });

    // === Signup Button Navigation ===
    connect(signupbtn, &QPushButton::clicked, this, &MainWindow::on_pushButton_3_clicked);

    // === ESC Shortcut (Global) ===
    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [=]() {
        clearCentralWidget();
    });
}


void MainWindow::on_pushButton_3_clicked()
{
    clearCentralWidget();

    QVBoxLayout *wrapperLayout = new QVBoxLayout;

    QFrame *formFrame = new QFrame;
    formFrame->setFixedSize(1200, 500);
    formFrame->setStyleSheet(
        "background-image: url('D:/project/admin_pannel/login_bg.png');"
        "background-repeat: no-repeat;"
        "background-position: center;"
        "border-radius: 20px;"
        );

    QGridLayout *gridLayout = new QGridLayout(formFrame);
    gridLayout->setContentsMargins(60, 80, 60, 80); // Inner padding
    gridLayout->setHorizontalSpacing(40);
    gridLayout->setVerticalSpacing(25);
    gridLayout->setColumnStretch(0, 1); // Label side (darker)
    gridLayout->setColumnStretch(1, 3); // Input side (lighter)

    // Create Labels
    QLabel *lblUsername = new QLabel("Username:");
    QLabel *lblPassword = new QLabel("Password:");
    QLabel *lblRetype   = new QLabel("Re-type Password:");
    QLabel *lblNIN      = new QLabel("NIN Number:");

    QString labelStyle = "color: white;";
    lblUsername->setStyleSheet(labelStyle);
    lblPassword->setStyleSheet(labelStyle);
    lblRetype->setStyleSheet(labelStyle);
    lblNIN->setStyleSheet(labelStyle);

    // Create LineEdits
    QLineEdit *editUsername = new QLineEdit;
    QLineEdit *editPassword = new QLineEdit;
    QLineEdit *editRetype   = new QLineEdit;
    QLineEdit *editNIN      = new QLineEdit;

    editUsername->setPlaceholderText("Enter your username");
    editPassword->setPlaceholderText("Enter password");
    editRetype->setPlaceholderText("Re-type password");
    editNIN->setPlaceholderText("Enter NIN Number");

    editPassword->setEchoMode(QLineEdit::Password);
    editRetype->setEchoMode(QLineEdit::Password);

    // LineEdit style
    QString lineEditStyle = R"(
    QLineEdit {
        background-image : url("D:/project/admin_pannel/username_bg");
        border: 1px ;
        border-radius: 8px;
        padding: 8px 12px;
        font-size: 14px;
    }
)";
    editUsername->setStyleSheet(lineEditStyle);
    editPassword->setStyleSheet(lineEditStyle);
    editRetype->setStyleSheet(lineEditStyle);
    editNIN->setStyleSheet(lineEditStyle);

    // LineEdit width
    editUsername->setFixedWidth(300);
    editPassword->setFixedWidth(300);
    editRetype->setFixedWidth(300);
    editNIN->setFixedWidth(300);

    // Add widgets row-by-row
    gridLayout->addWidget(lblUsername, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(editUsername, 0, 1);

    gridLayout->addWidget(lblPassword, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(editPassword, 1, 1);

    gridLayout->addWidget(lblRetype, 2, 0, Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(editRetype, 2, 1);

    gridLayout->addWidget(lblNIN, 3, 0, Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(editNIN, 3, 1);

    // Signup button
    QPushButton *signupBtn = new QPushButton("Sign Up");
    signupBtn->setStyleSheet(R"(
    QPushButton {
        background-image: url("D:/project/admin_pannel/login.png");

        padding: 10px 20px;
        font-size: 14px;
        border-radius: 8px;
    }
    QPushButton:hover {
         background-image: url("D:/project/admin_pannel/username_bg.png");
    }
)");
    //signupBtn->setStyleSheet("background-image: url(\"D:/project/admin_pannel/login.png\");");
    signupBtn->setFixedWidth(140);

    // Add button below inputs (aligned to right)
    gridLayout->addWidget(signupBtn, 4, 1, Qt::AlignVCenter);

    // Focus movement
    connect(editUsername, &QLineEdit::returnPressed, editPassword, QOverload<>::of(&QLineEdit::setFocus));
    connect(editPassword, &QLineEdit::returnPressed, editRetype, QOverload<>::of(&QLineEdit::setFocus));
    connect(editRetype, &QLineEdit::returnPressed, editNIN, QOverload<>::of(&QLineEdit::setFocus));
    connect(editNIN, &QLineEdit::returnPressed, signupBtn, &QPushButton::click);

    // Center the formFrame in main layout
    wrapperLayout->addStretch();
    wrapperLayout->addWidget(formFrame, 0, Qt::AlignCenter);
    wrapperLayout->addStretch();
    ui->centralwidget->setLayout(wrapperLayout);

    // ESC Shortcut to clear
    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, &MainWindow::clearCentralWidget);

    // Focus on username at startup
    QTimer::singleShot(0, editUsername, SLOT(setFocus()));

    // Signup logic (unchanged)
    connect(signupBtn, &QPushButton::clicked, this, [=]() {
        QString username = editUsername->text();
        QString password = editPassword->text();
        QString repass = editRetype->text();
        QString nin = editNIN->text();

        if (username.isEmpty() || password.isEmpty() || repass.isEmpty() || nin.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please fill in all fields.");
            return;
        }
        if (password != repass) {
            QMessageBox::warning(this, "Error", "Passwords do not match.");
            return;
        }

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "signup_conn");
        db.setDatabaseName("D:/project/database/login_info.db");

        if (!db.open()) {
            QMessageBox::warning(this, "Error", "DB Open Failed: " + db.lastError().text());
            return;
        }

        QSqlQuery query(db);
        query.prepare("INSERT INTO login (username, password, NIN) VALUES (:u, :p, :n)");
        query.bindValue(":u", username);
        query.bindValue(":p", password);
        query.bindValue(":n", nin);

        if (query.exec()) {
            QMessageBox::information(this, "Signup", "Account created successfully!");
            db.close();
            on_pushButton_2_clicked();
        } else {
            QMessageBox::warning(this, "Error", "Signup Failed: " + query.lastError().text());
            db.close();
        }
    });
}


void MainWindow::on_addbus_clicked()
{
    if (!login) {
        on_pushButton_2_clicked();
        return;
    }


    QString from_ui = ui->lineEdit->text();
    QString to_ui = ui->lineEdit_2->text();


    clearCentralWidget();

    QVBoxLayout *mainLayout = new QVBoxLayout;

    QFrame *formFrame = new QFrame;
    formFrame->setFixedSize(400, 450);
    formFrame->setStyleSheet("background: #77AABF; border-radius: 20px;");

    QVBoxLayout *formLayout = new QVBoxLayout(formFrame);

    QLineEdit *fromEdit = new QLineEdit(from_ui);
    QLineEdit *toEdit = new QLineEdit(to_ui);
    QLineEdit *time = new QLineEdit;
    QLineEdit *busid = new QLineEdit;

    connect(fromEdit, &QLineEdit::returnPressed, toEdit, QOverload<>::of(&QLineEdit::setFocus));
    connect(toEdit, &QLineEdit::returnPressed, time, QOverload<>::of(&QLineEdit::setFocus));
    connect(time, &QLineEdit::returnPressed, busid, QOverload<>::of(&QLineEdit::setFocus));
    time->setPlaceholderText("hour:minute in 24 hour format");
    QTimer::singleShot(0, time, SLOT(setFocus()));
    busid->setPlaceholderText("Number plate of your Bus");

    QPushButton *saveBtn = new QPushButton("Add Bus");
    connect(busid, &QLineEdit::returnPressed, saveBtn, &QPushButton::click);
    formLayout->addWidget(new QLabel("From:"));
    formLayout->addWidget(fromEdit);
    formLayout->addWidget(new QLabel("To:"));
    formLayout->addWidget(toEdit);
    formLayout->addWidget(new QLabel("Time:"));
    formLayout->addWidget(time);
    formLayout->addWidget(new QLabel("Bus ID:"));
    formLayout->addWidget(busid);
    formLayout->addSpacing(20);
    formLayout->addWidget(saveBtn, 0, Qt::AlignCenter);

    mainLayout->addStretch();
    mainLayout->addWidget(formFrame, 0, Qt::AlignCenter);
    mainLayout->addStretch();


    ui->centralwidget->setLayout(mainLayout);
    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [=]() {
        clearCentralWidget();
        // You can also restore the original screen here if needed
        // For example, reload the welcome screen or input screen
    });

    connect(saveBtn, &QPushButton::clicked, this, [=]() {

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "addbus_conn");
        db.setDatabaseName("D:/project/database/addbus.db");

        if (!db.open()) {
            QMessageBox::warning(this, "DB Error", db.lastError().text());
            return;
        }

        QSqlQuery query(db);
        query.prepare("INSERT INTO Bus (\"From\", \"To\", \"Time\", \"BusId\", \"Username\") "
                      "VALUES (:f, :t, :time, :id, :usr)");
        query.bindValue(":f", fromEdit->text());
        query.bindValue(":t", toEdit->text());
        query.bindValue(":time", time->text());
        query.bindValue(":id", busid->text());
        query.bindValue(":usr", currentUsername);

        if (query.exec()) {
            QMessageBox::information(this, "Inserted", "Bus added successfully");
            db.close();
            clearCentralWidget();
            showWelcomeUI(currentUsername);
        } else {
            QMessageBox::warning(this, "Insert Failed", query.lastError().text());
        }


        checkBus();
    });

}
