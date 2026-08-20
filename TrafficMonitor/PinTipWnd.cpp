#include "stdafx.h"
#include "TrafficMonitor.h"
#include "PinTipWnd.h"
#include "StrTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPinTipWnd, CWnd)

LPCTSTR CPinTipWnd::CLASS_NAME = _T("TrafficMonitorPinTipWndClass");

BEGIN_MESSAGE_MAP(CPinTipWnd, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

CPinTipWnd::CPinTipWnd()
{
}

CPinTipWnd::~CPinTipWnd()
{
    if (m_font_created)
        m_font.DeleteObject();
}

BOOL CPinTipWnd::Create(CWnd* pParentWnd, CPoint pt)
{
    //注册自定义窗口类（只需注册一次）
    WNDCLASS wndcls{};
    HINSTANCE hInst{ AfxGetInstanceHandle() };
    if (!(::GetClassInfo(hInst, CLASS_NAME, &wndcls)))
    {
        wndcls.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wndcls.lpfnWndProc = ::DefWindowProc;
        wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
        wndcls.hInstance = hInst;
        wndcls.hIcon = NULL;
        wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wndcls.hbrBackground = NULL;
        wndcls.lpszMenuName = NULL;
        wndcls.lpszClassName = CLASS_NAME;

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
            return FALSE;
        }
    }

    //设置字体
    if (!m_font_created)
    {
        LOGFONT lf{};
        NONCLIENTMETRICS ncm{};
        ncm.cbSize = sizeof(NONCLIENTMETRICS);
        if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
            lf = ncm.lfMessageFont;
        else
        {
            lf.lfHeight = -12;
            _tcscpy_s(lf.lfFaceName, _T("Segoe UI"));
        }
        m_font.CreateFontIndirect(&lf);
        m_font_created = TRUE;
    }

    CRect rect(pt.x, pt.y, pt.x + 260, pt.y + 80);
    ClampToWorkArea(rect);

    //WS_EX_TOPMOST：始终置顶；WS_EX_TOOLWINDOW：不在任务栏和Alt+Tab中显示
    DWORD ex_style{ WS_EX_TOPMOST | WS_EX_TOOLWINDOW };
    DWORD style{ WS_POPUP };

    BOOL ret{ CWnd::CreateEx(ex_style, CLASS_NAME, _T("TrafficMonitor"), style,
        rect.left, rect.top, rect.Width(), rect.Height(), pParentWnd->GetSafeHwnd(), NULL) };

    return ret;
}

void CPinTipWnd::SetTipText(const CString& text)
{
    m_tip_text = text;
    if (GetSafeHwnd() != NULL)
    {
        UpdateWindowSize();
        Invalidate(FALSE);
    }
}

void CPinTipWnd::ShowPin()
{
    if (GetSafeHwnd() == NULL)
        return;

    if (!IsWindowVisible())
    {
        //窗口当前处于隐藏状态，在鼠标附近重新显示
        CPoint pt;
        ::GetCursorPos(&pt);
        CRect rect;
        GetWindowRect(&rect);
        rect.MoveToXY(pt.x, pt.y);
        ClampToWorkArea(rect);
        SetWindowPos(&wndTopMost, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
    }
    else
    {
        //窗口已经可见，只需要保证它在最顶层，不移动位置
        SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    }
    Invalidate(FALSE);
}

void CPinTipWnd::HidePin()
{
    if (GetSafeHwnd() != NULL)
        ShowWindow(SW_HIDE);
}

BOOL CPinTipWnd::IsPinVisible() const
{
    return (GetSafeHwnd() != NULL && IsWindowVisible());
}

void CPinTipWnd::UpdateWindowSize()
{
    CClientDC dc(this);
    CFont* old_font{ dc.SelectObject(&m_font) };

    CRect calc_rect(0, 0, MAX_WIDTH, 0);
    dc.DrawText(m_tip_text, calc_rect, DT_LEFT | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);

    dc.SelectObject(old_font);

    int width{ max(calc_rect.Width() + PADDING * 2, CLOSE_BTN_SIZE + PADDING * 3) };
    int height{ calc_rect.Height() + PADDING * 2 + HEADER_HEIGHT };

    CRect wnd_rect;
    GetWindowRect(&wnd_rect);
    if (wnd_rect.Width() != width || wnd_rect.Height() != height)
    {
        SetWindowPos(NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void CPinTipWnd::ClampToWorkArea(CRect& rect) const
{
    CRect work_area;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);

    int width{ rect.Width() };
    int height{ rect.Height() };

    int left{ rect.left };
    int top{ rect.top };

    if (left + width > work_area.right)
        left = work_area.right - width;
    if (top + height > work_area.bottom)
        top = work_area.bottom - height;
    if (left < work_area.left)
        left = work_area.left;
    if (top < work_area.top)
        top = work_area.top;

    rect.SetRect(left, top, left + width, top + height);
}

CRect CPinTipWnd::GetCloseButtonRect() const
{
    CRect rect;
    GetClientRect(&rect);
    return CRect(rect.right - CLOSE_BTN_SIZE - 4, 4, rect.right - 4, 4 + CLOSE_BTN_SIZE);
}

void CPinTipWnd::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);

    //使用内存DC双缓冲绘制，避免闪烁
    CDC mem_dc;
    mem_dc.CreateCompatibleDC(&dc);
    CBitmap bmp;
    bmp.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
    CBitmap* old_bmp{ mem_dc.SelectObject(&bmp) };

    COLORREF bk_color{ ::GetSysColor(COLOR_INFOBK) };
    COLORREF text_color{ ::GetSysColor(COLOR_INFOTEXT) };
    COLORREF border_color{ ::GetSysColor(COLOR_WINDOWFRAME) };

    CBrush bk_brush(bk_color);
    mem_dc.FillRect(rect, &bk_brush);

    CPen border_pen(PS_SOLID, 1, border_color);
    CPen* old_pen{ mem_dc.SelectObject(&border_pen) };
    CBrush* old_brush{ (CBrush*)mem_dc.SelectObject(GetStockObject(NULL_BRUSH)) };
    CRect border_rect(rect);
    border_rect.DeflateRect(0, 0, 1, 1);
    mem_dc.Rectangle(border_rect);
    mem_dc.SelectObject(old_brush);
    mem_dc.SelectObject(old_pen);

    //绘制文字
    CFont* old_font{ mem_dc.SelectObject(&m_font) };
    mem_dc.SetBkMode(TRANSPARENT);
    mem_dc.SetTextColor(text_color);
    CRect text_rect(rect);
    text_rect.top += HEADER_HEIGHT;
    text_rect.DeflateRect(PADDING, 0, PADDING, PADDING);
    mem_dc.DrawText(m_tip_text, text_rect, DT_LEFT | DT_NOPREFIX | DT_WORDBREAK);
    mem_dc.SelectObject(old_font);

    //绘制关闭按钮（“×”）
    CRect close_rect{ GetCloseButtonRect() };
    mem_dc.SetTextColor(text_color);
    mem_dc.DrawText(_T("\u00D7"), close_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    dc.BitBlt(0, 0, rect.Width(), rect.Height(), &mem_dc, 0, 0, SRCCOPY);
    mem_dc.SelectObject(old_bmp);
}

BOOL CPinTipWnd::OnEraseBkgnd(CDC* pDC)
{
    //已经在OnPaint中通过双缓冲绘制了整个背景，这里不需要再擦除，避免闪烁
    return TRUE;
}

void CPinTipWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect close_rect{ GetCloseButtonRect() };
    if (close_rect.PtInRect(point))
    {
        HidePin();
        return;
    }

    //使窗口可以在没有标题栏的情况下被拖动
    //原理：向窗口发送SC_MOVE系统命令，让系统进入非客户区拖动循环
    ReleaseCapture();
    SendMessage(WM_SYSCOMMAND, SC_MOVE | 0x0002 /*HTCAPTION*/, 0);
}

void CPinTipWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
    CMenu menu;
    menu.CreatePopupMenu();

    CString unpin_text{ theApp.m_str_table.LoadMenuText(L"TXT_UNPIN_TOOLTIP").c_str() };
    if (unpin_text.IsEmpty())
        unpin_text = _T("Unpin");
    menu.AppendMenu(MF_STRING, 1, unpin_text);

    CPoint screen_pt(point);
    ClientToScreen(&screen_pt);
    int cmd{ menu.TrackPopupMenu(TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, screen_pt.x, screen_pt.y, this) };
    if (cmd == 1)
        HidePin();
}

void CPinTipWnd::OnDestroy()
{
    CWnd::OnDestroy();
}
