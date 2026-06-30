#include "nsc_qt/widgets/register_widget.h"
#include "mips/decoder.h"
#include "mips/registers.h"

#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <algorithm>

namespace nsc::qt {

namespace {

static QColor lerp_color(QColor a, QColor b, float t) {
    return QColor(
        static_cast<int>(a.red()   + (b.red()   - a.red())   * t),
        static_cast<int>(a.green() + (b.green() - a.green()) * t),
        static_cast<int>(a.blue()  + (b.blue()  - a.blue())  * t)
    );
}

} // anonymous namespace

RegisterWidget::RegisterWidget(QWidget* parent) : QWidget(parent)
{
    buildGrid();
}

void RegisterWidget::buildGrid()
{
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(4, 4, 4, 4);
    outer->setSpacing(2);

    auto* title = new QLabel("Registers", this);
    title->setAlignment(Qt::AlignCenter);
    QFont tf = title->font();
    tf.setBold(true);
    title->setFont(tf);
    outer->addWidget(title);

    grid_ = new QGridLayout;
    grid_->setSpacing(2);
    outer->addLayout(grid_);

    // 4 columns × 8 rows = 32 registers
    for (int i = 0; i < 32; ++i) {
        const int col = i / 8;
        const int row = i % 8;

        auto* frame = new QWidget(this);
        frame->setAutoFillBackground(true);
        auto* fl = new QVBoxLayout(frame);
        fl->setContentsMargins(3, 1, 3, 1);
        fl->setSpacing(0);

        cells_[i].name  = new QLabel(frame);
        cells_[i].value = new QLabel(frame);

        cells_[i].name->setFont(QFont("monospace", 8));
        cells_[i].value->setFont(QFont("monospace", 8));
        cells_[i].value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        fl->addWidget(cells_[i].name);
        fl->addWidget(cells_[i].value);

        grid_->addWidget(frame, row, col);
        updateCell(i);
    }
}

void RegisterWidget::updateCell(int idx)
{
    const auto& c  = cells_[idx];
    const uint8_t u = static_cast<uint8_t>(idx);

    // Name label
    QString name_str = QString("$%1").arg(idx);
    if (show_aliases_ && idx < 32)
        name_str += QString(" (%1)").arg(QString::fromStdString(
            std::string(mips::register_abi_name(u))));
    c.name->setText(name_str);

    // Value label
    c.value->setText(QString("0x%1").arg(values_[idx], 8, 16, QChar('0')));

    // Background colour
    QColor bg;
    if (idx == 0) {
        bg = dark_mode_ ? QColor(0x30,0x30,0x30) : QColor(0xDD,0xDD,0xDD);
    } else if (cells_[idx].fade > 0) {
        const float t = 1.0f - static_cast<float>(cells_[idx].fade) / 5.0f;
        const QColor green(0x80, 0xFF, 0x80);
        const QColor normal = dark_mode_ ? QColor(0x2A,0x2A,0x2A) : Qt::white;
        bg = lerp_color(green, normal, t);
    } else if (static_cast<uint8_t>(idx) == read_rs_ || static_cast<uint8_t>(idx) == read_rt_) {
        bg = dark_mode_ ? QColor(0x00,0x60,0x60) : QColor(0xB2,0xEB,0xF2);
    } else {
        bg = dark_mode_ ? QColor(0x2A,0x2A,0x2A) : Qt::white;
    }

    QPalette pal = c.name->parentWidget()->palette();
    pal.setColor(QPalette::Window, bg);
    c.name->parentWidget()->setPalette(pal);
}

void RegisterWidget::setPipelineState(const mips::PipelineState& state)
{
    // Decay all fade counters first
    for (int i = 1; i < 32; ++i)
        if (cells_[i].fade > 0) --cells_[i].fade;

    // Detect a register written in WB
    const auto& wb = state.stages[4];
    if (wb.valid && !wb.stalled && !wb.flushed && wb.raw != 0) {
        auto decoded = mips::Decoder::decode(wb.raw);
        if (decoded) {
            uint8_t dest = 0xFF;
            const auto& d = *decoded;
            if (d.format == mips::InstrFormat::R) {
                dest = d.r().rd;
            } else if (d.format == mips::InstrFormat::I) {
                if (d.opcode != mips::Opcode::SW && d.opcode != mips::Opcode::BEQ &&
                    d.opcode != mips::Opcode::BNE)
                    dest = d.i().rt;
            } else if (d.opcode == mips::Opcode::JAL) {
                dest = 31; // $ra
            }
            if (dest != 0xFF && dest != 0 && dest < 32) {
                cells_[dest].fade = 5;
            }
        }
    }

    // Detect registers read in ID
    read_rs_ = 0xFF;
    read_rt_ = 0xFF;
    const auto& id = state.stages[1];
    if (id.valid && id.raw != 0) {
        auto decoded = mips::Decoder::decode(id.raw);
        if (decoded) {
            const auto& d = *decoded;
            if (d.format == mips::InstrFormat::R) {
                read_rs_ = d.r().rs;
                read_rt_ = d.r().rt;
            } else if (d.format == mips::InstrFormat::I) {
                read_rs_ = d.i().rs;
                if (d.opcode == mips::Opcode::SW || d.opcode == mips::Opcode::BEQ ||
                    d.opcode == mips::Opcode::BNE)
                    read_rt_ = d.i().rt;
            }
        }
    }

    // Update displayed values from WB-committed register file snapshot.
    // We don't have direct register file access here; values are updated by
    // MainWindow via pipelineStateChanged which routes through the controller.
    // For now just refresh all cells to re-apply highlights.
    for (int i = 0; i < 32; ++i)
        updateCell(i);
}

void RegisterWidget::setShowAliases(bool show)
{
    show_aliases_ = show;
    for (int i = 0; i < 32; ++i) updateCell(i);
}

void RegisterWidget::setDarkMode(bool dark)
{
    dark_mode_ = dark;
    for (int i = 0; i < 32; ++i) updateCell(i);
}

void RegisterWidget::clear()
{
    values_.fill(0);
    read_rs_ = 0xFF;
    read_rt_ = 0xFF;
    for (auto& c : cells_) c.fade = 0;
    for (int i = 0; i < 32; ++i) updateCell(i);
}

void RegisterWidget::updateValues(const std::array<uint32_t, 32>& vals)
{
    values_ = vals;
    for (int i = 0; i < 32; ++i) updateCell(i);
}

} // namespace nsc::qt
