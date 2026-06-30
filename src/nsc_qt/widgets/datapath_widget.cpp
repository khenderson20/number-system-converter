#include "nsc_qt/widgets/datapath_widget.h"
#include "mips/decoder.h"
#include "mips/registers.h"

#include <QContextMenuEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QString>
#include <QVBoxLayout>
#include <algorithm>
#include <sstream>

namespace nsc::qt {

namespace {

static const QColor STAGE_COLORS_LIGHT[5] = {
    QColor("#E3F2FD"),  // IF  – light blue
    QColor("#E0F7FA"),  // ID  – light cyan
    QColor("#E8F5E9"),  // EX  – light green
    QColor("#FFFDE7"),  // MEM – light yellow
    QColor("#FFEBEE"),  // WB  – light pink/red
};
static const QColor STAGE_COLORS_DARK[5] = {
    QColor("#0D47A1"),
    QColor("#006064"),
    QColor("#1B5E20"),
    QColor("#F57F17"),
    QColor("#B71C1C"),
};

static const char* STAGE_NAMES[5] = {"IF", "ID", "EX", "MEM", "WB"};

// Format a decoded instruction as a human-readable string.
static std::string format_instr(uint32_t raw) {
    using namespace mips;
    auto decoded = Decoder::decode(raw);
    if (!decoded) return "(?/?)";
    const auto& d = *decoded;
    std::string mn = std::string(Decoder::mnemonic(d));
    std::ostringstream os;
    os << mn << " ";
    switch (d.format) {
    case InstrFormat::R: {
        const auto& r = d.r();
        if (d.opcode == Opcode::SPECIAL) {
            using F = FunctCode;
            if (r.funct == F::SLL || r.funct == F::SRL || r.funct == F::SRA) {
                os << "$" << register_abi_name(r.rd)
                   << ", $" << register_abi_name(r.rt)
                   << ", " << +r.shamt;
            } else if (r.funct == F::JR) {
                os << "$" << register_abi_name(r.rs);
            } else if (r.funct == F::JALR) {
                os << "$" << register_abi_name(r.rd)
                   << ", $" << register_abi_name(r.rs);
            } else {
                os << "$" << register_abi_name(r.rd)
                   << ", $" << register_abi_name(r.rs)
                   << ", $" << register_abi_name(r.rt);
            }
        }
        break;
    }
    case InstrFormat::I: {
        const auto& i = d.i();
        if (d.opcode == Opcode::LW || d.opcode == Opcode::LBU ||
            d.opcode == Opcode::LHU || d.opcode == Opcode::SW) {
            os << "$" << register_abi_name(i.rt) << ", " << static_cast<int16_t>(i.imm)
               << "($" << register_abi_name(i.rs) << ")";
        } else if (d.opcode == Opcode::LUI) {
            os << "$" << register_abi_name(i.rt)
               << ", 0x" << std::hex << i.imm;
        } else if (d.opcode == Opcode::BEQ || d.opcode == Opcode::BNE) {
            os << "$" << register_abi_name(i.rs)
               << ", $" << register_abi_name(i.rt)
               << ", " << static_cast<int16_t>(i.imm);
        } else {
            os << "$" << register_abi_name(i.rt)
               << ", $" << register_abi_name(i.rs)
               << ", " << static_cast<int16_t>(i.imm);
        }
        break;
    }
    case InstrFormat::J: {
        const auto& j = d.j();
        os << "0x" << std::hex << j.target;
        break;
    }
    default: break;
    }
    return os.str();
}

} // anonymous namespace

DatapathWidget::DatapathWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(5 * BOX_W + 4 * GAP + 40, BOX_H + MARGIN_Y * 2 + 60);
}

void DatapathWidget::setPipelineState(const mips::PipelineState& state)
{
    state_ = state;
    update();
}

void DatapathWidget::setBreakpoints(const std::unordered_set<uint32_t>& bps)
{
    breakpoints_ = bps;
    update();
}

void DatapathWidget::setDarkMode(bool dark)
{
    dark_mode_ = dark;
    update();
}

void DatapathWidget::initializeGL()
{
    initializeOpenGLFunctions();
}

void DatapathWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void DatapathWidget::paintGL()
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    p.fillRect(rect(), dark_mode_ ? QColor(0x1E, 0x1E, 0x1E) : QColor(0xF5, 0xF5, 0xF5));

    // Forwarding arrows (behind boxes)
    drawForwardingArrows(p);

    // Stage boxes
    for (int i = 0; i < 5; ++i)
        drawStageBox(p, i, state_.stages[static_cast<std::size_t>(i)]);

    // Cycle label
    p.setPen(dark_mode_ ? Qt::white : Qt::black);
    p.setFont(QFont("monospace", 9));
    p.drawText(4, height() - 4,
               QString("Cycle: %1").arg(static_cast<qulonglong>(state_.cycle)));
}

QRect DatapathWidget::stageRect(int idx) const
{
    const int total_w = 5 * BOX_W + 4 * GAP;
    const int x0 = (width() - total_w) / 2;
    const int y0 = MARGIN_Y;
    return {x0 + idx * (BOX_W + GAP), y0, BOX_W, BOX_H};
}

int DatapathWidget::stageAtPos(QPoint pos) const
{
    for (int i = 0; i < 5; ++i)
        if (stageRect(i).contains(pos)) return i;
    return -1;
}

void DatapathWidget::drawStageBox(QPainter& p, int idx,
                                   const mips::StageSnapshot& snap) const
{
    const QRect r = stageRect(idx);
    const QColor& bg = dark_mode_ ? STAGE_COLORS_DARK[idx] : STAGE_COLORS_LIGHT[idx];

    // Box fill
    p.setBrush(snap.valid ? bg : (dark_mode_ ? QColor(0x30,0x30,0x30) : QColor(0xEE,0xEE,0xEE)));
    p.setPen(QPen(dark_mode_ ? Qt::gray : Qt::darkGray, 1));
    p.drawRoundedRect(r, 6, 6);

    // Stage name header band
    const QRect header_r(r.x(), r.y(), r.width(), 22);
    p.setBrush(dark_mode_ ? QColor(0x00,0x00,0x00,80) : QColor(0x00,0x00,0x00,20));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(header_r, 6, 6);
    p.drawRect(QRect(header_r.x(), header_r.y() + 6, header_r.width(), 16));

    p.setPen(dark_mode_ ? Qt::white : Qt::black);
    QFont hf("monospace", 10, QFont::Bold);
    p.setFont(hf);
    p.drawText(header_r, Qt::AlignCenter, STAGE_NAMES[idx]);

    if (!snap.valid) {
        p.setPen(dark_mode_ ? QColor(0x80,0x80,0x80) : QColor(0x99,0x99,0x99));
        p.setFont(QFont("monospace", 8));
        p.drawText(r.adjusted(0, 24, 0, 0), Qt::AlignCenter,
                   snap.stalled ? "STALL" : snap.flushed ? "FLUSH" : "---");
        return;
    }

    // PC
    p.setPen(dark_mode_ ? QColor(0xBB,0xBB,0xFF) : QColor(0x33,0x33,0x99));
    p.setFont(QFont("monospace", 8));
    p.drawText(r.adjusted(4, 26, -4, 0),
               Qt::AlignTop | Qt::AlignLeft,
               QString("PC: 0x%1").arg(snap.pc, 8, 16, QChar('0')));

    // Stall/Flush badge
    if (snap.stalled || snap.flushed) {
        const QString badge = snap.stalled ? "STALL" : "FLUSH";
        p.setPen(snap.stalled ? QColor("#E65100") : QColor("#880E4F"));
        p.setFont(QFont("monospace", 7, QFont::Bold));
        p.drawText(r.adjusted(4, 26, -4, 0),
                   Qt::AlignTop | Qt::AlignRight, badge);
    }

    // Decoded instruction
    if (snap.raw != 0) {
        const std::string decoded = format_instr(snap.raw);
        p.setPen(dark_mode_ ? Qt::white : Qt::black);
        p.setFont(QFont("monospace", 8));
        p.drawText(r.adjusted(4, 42, -4, -4),
                   Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                   QString::fromStdString(decoded));
    }

    // Breakpoint indicator (red circle)
    if (breakpoints_.count(snap.pc)) {
        p.setBrush(Qt::red);
        p.setPen(Qt::NoPen);
        p.drawEllipse(r.x() + r.width() - 14, r.y() + 4, 10, 10);
    }
}

void DatapathWidget::drawForwardingArrows(QPainter& p) const
{
    // Draw a subtle arc above the pipeline boxes for each active forwarding path.
    auto drawArrow = [&](int from_idx, int to_idx, QColor color) {
        const QRect from_r = stageRect(from_idx);
        const QRect to_r   = stageRect(to_idx);
        const int fy = from_r.top() - 12;
        const QPoint start(from_r.center().x(), fy);
        const QPoint end(to_r.center().x(), fy);
        QPainterPath path;
        path.moveTo(start);
        path.cubicTo(start + QPoint(0, -18), end + QPoint(0, -18), end);
        p.setPen(QPen(color, 2, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);
    };

    if (state_.fwd_ex_to_ex_a || state_.fwd_ex_to_ex_b)
        drawArrow(3, 2, QColor("#FF6F00")); // EX/MEM → EX
    if (state_.fwd_mem_to_ex_a || state_.fwd_mem_to_ex_b)
        drawArrow(4, 2, QColor("#7B1FA2")); // MEM/WB → EX
}

void DatapathWidget::mousePressEvent(QMouseEvent* ev)
{
    QOpenGLWidget::mousePressEvent(ev);
}

void DatapathWidget::mouseDoubleClickEvent(QMouseEvent* ev)
{
    const int idx = stageAtPos(ev->pos());
    if (idx < 0) return;
    const auto& snap = state_.stages[static_cast<std::size_t>(idx)];
    if (!snap.valid) return;
    emit stageDetailRequested(idx, snap.pc, snap.raw);
}

void DatapathWidget::contextMenuEvent(QContextMenuEvent* ev)
{
    const int idx = stageAtPos(ev->pos());
    if (idx < 0) return;
    const auto& snap = state_.stages[static_cast<std::size_t>(idx)];
    if (!snap.valid) return;

    QMenu menu(this);
    const bool has_bp = breakpoints_.count(snap.pc) > 0;
    QAction* act = menu.addAction(has_bp ? "Clear Breakpoint" : "Set Breakpoint");
    if (menu.exec(ev->globalPos()) == act)
        emit breakpointToggleRequested(snap.pc);
}

} // namespace nsc::qt
