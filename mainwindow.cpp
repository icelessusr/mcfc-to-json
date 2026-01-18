#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <optional>
#include <string>
#include <sstream>
#include "json.hpp"
#include <QApplication>
#include <QClipboard>
#include <QString>

QClipboard* clipboard = QApplication::clipboard();
using namespace nlohmann;

struct Object {
    std::optional<std::string> color;
    std::optional<bool> bold;
    std::optional<bool> italic;
    std::optional<bool> underlined;
    std::optional<bool> strikethrough;
    std::optional<bool> obfuscated;
    std::optional<std::string> text;

    std::string getColor() const { return color.value_or("dark_purple"); }
    bool getBold() const { return bold.value_or(false); }
    bool getItalic() const { return italic.value_or(false); }
    bool getUnderlined() const { return underlined.value_or(false); }
    bool getStrikethrough() const { return strikethrough.value_or(false); }
    bool getObfuscated() const { return obfuscated.value_or(false); }
    std::string getText() const { return text.value_or(""); }
};

Object overwriteObject(const Object& oldObject, const Object& newObject) {
    Object result = oldObject;

    if (newObject.color.has_value()) result.color = newObject.color;
    if (newObject.bold.has_value()) result.bold = newObject.bold;
    if (newObject.italic.has_value()) result.italic = newObject.italic;
    if (newObject.underlined.has_value()) result.underlined = newObject.underlined;
    if (newObject.strikethrough.has_value()) result.strikethrough = newObject.strikethrough;
    if (newObject.obfuscated.has_value()) result.obfuscated = newObject.obfuscated;
    if (newObject.text.has_value()) result.text = newObject.text;

    return result;
}

const Object defaultObject;
const Object preferObject{
    std::nullopt,
    std::nullopt,
    false,
    std::nullopt,
    std::nullopt,
    std::nullopt,
    std::nullopt
};

Object ObjectClean(const Object& object) {
    Object out;

    if (object.getColor() != defaultObject.getColor()) out.color = object.color;
    if (object.getBold() != defaultObject.getBold()) out.bold = object.bold;
    if (object.getItalic() != defaultObject.getItalic()) out.italic = object.italic;
    if (object.getUnderlined() != defaultObject.getUnderlined()) out.underlined = object.underlined;
    if (object.getStrikethrough() != defaultObject.getStrikethrough()) out.strikethrough = object.strikethrough;
    if (object.getObfuscated() != defaultObject.getObfuscated()) out.obfuscated = object.obfuscated;

    out.text = object.text;

    return out;
}

const std::unordered_map<char, std::string> colorTable = {
    {'0', "black"},
    {'1', "dark_blue"},
    {'2', "dark_green"},
    {'3', "dark_aqua"},
    {'4', "dark_red"},
    {'5', "dark_purple"},
    {'6', "gold"},
    {'7', "gray"},
    {'8', "dark_gray"},
    {'9', "blue"},
    {'a', "green"},
    {'b', "aqua"},
    {'c', "red"},
    {'d', "light_purple"},
    {'e', "yellow"},
    {'f', "white"}
};

std::vector<std::string> split(const std::string text, char delimiter = '\n') {
    std::vector<std::string> lines;
    std::string line;
    std::stringstream stringstream(text);

    while (std::getline(stringstream, line, delimiter)) {
        lines.push_back(line);
    }

    return lines;
}

using Line = std::vector<Object>;
using Lines = std::vector<Line>;

json json_objectToLine(const Object& object) {
    json j;

    if (object.color.has_value()) j["color"] = object.color.value();
    if (object.bold.has_value()) j["bold"] = object.bold.value();
    if (object.italic.has_value()) j["italic"] = object.italic.value();
    if (object.underlined.has_value()) j["underlined"] = object.underlined.value();
    if (object.strikethrough.has_value()) j["strikethrough"] = object.strikethrough.value();
    if (object.obfuscated.has_value()) j["obfuscated"] = object.obfuscated.value();

    j["text"] = object.getText();
    return j;
}

json json_LineToLines(const Lines& lines) {
    json j = json::array();

    for (const auto& line : lines) {
        json lineJson = json::array();
        for (const auto& object : line) {
            lineJson.push_back(json_objectToLine(object));
        }
        j.push_back(lineJson);
    }

    return j;
}

Lines convert(
    const std::string text,
    const Object& preferObject,
    bool resetWhenColor,
    bool resetWhenLine
) {
    auto textLines = split(text);
    Lines lines;

    Object resetObject = overwriteObject(defaultObject, preferObject);
    Object object = resetObject;

    for ( const auto& textLine : textLines ) {
        if (resetWhenLine) object = resetObject;

        Line line;
        std::string tempText = "";

        int skip = 0;

        for ( size_t i = 0; i < textLine.size(); ++i ) {
            char letter = textLine[i];

            if ( skip > 0 ) { skip--; continue; }

            if ( letter == '&' && i + 1 < textLine.size()) {
                if ( tempText != "" ) {
                    Object temp = object;
                    temp.text = tempText;
                    line.push_back(ObjectClean(temp));
                    tempText.clear();
                }

                char code = textLine[i + 1];

                if ( code == '#' ) {
                    if (resetWhenColor) object = resetObject;
                    object.color = textLine.substr(i+1, 7);
                    skip = 7;
                }
                else if ( code == 'l' ) { object.bold = true; skip = 1; }
                else if ( code == 'o' ) { object.italic = true; skip = 1; }
                else if ( code == 'n' ) { object.underlined = true; skip = 1; }
                else if ( code == 'm' ) { object.strikethrough = true; skip = 1; }
                else if ( code == 'k' ) { object.obfuscated = true; skip = 1; }
                else if ( code == 'r' ) { object = resetObject; skip = 1; }
                else if ( colorTable.count(code) ) {
                    if (resetWhenColor) object = resetObject;
                    object.color = colorTable.at(code);
                    skip = 1;
                }
            } else {
                tempText += letter;
            }
        }

        Object temp = object;
        temp.text = tempText;
        line.push_back(ObjectClean(temp));
        tempText.clear();
        lines.push_back(line);
    }

    return lines;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::on_buttonConvert_clicked()
{
    std::string source = ui->textInput->toPlainText().toStdString();

    Lines converted = convert(
        source,
        preferObject,
        ui->actionEveryColorChange->isChecked(),
        ui->actionEveryNewLine->isChecked()
    );
    json jsoned = json_LineToLines(converted);
    std::string stringed = jsoned.dump();
    std::cout << stringed << std::endl;
    clipboard->setText(QString::fromStdString(stringed));
}

void MainWindow::on_actionClearTextInput_triggered() {
    ui->textInput->clear();
}


void MainWindow::on_actionConvert_triggered() {
    MainWindow::on_buttonConvert_clicked();
}


void MainWindow::on_buttonClear_clicked() {
    MainWindow::on_actionClearTextInput_triggered();
}


void MainWindow::on_actionAboutMCFCToJson_triggered()
{

}

