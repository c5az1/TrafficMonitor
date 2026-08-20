#pragma once

// CPinTipWnd
// 一个可以固定（钉住）在桌面上的提示信息窗口。
// 用于将主窗口的鼠标提示信息（Traffic Used Today / Upload / Download 等）
// 以一个不会自动消失的独立小窗口的形式显示出来。
//
// 使用方式：
//   1. 调用 Create() 创建窗口（只需创建一次）。
//   2. 调用 SetTipText() 设置/刷新要显示的文本内容。
//   3. 调用 ShowPin() 在鼠标当前位置附近显示该窗口（如果已经显示，则只更新内容，不移动位置）。
//   4. 用户可以按住鼠标左键拖动窗口到任意位置。
//   5. 用户点击窗口右上角的关闭按钮，或者右键菜单中的“取消固定”，可以隐藏该窗口。
//
// 该类不会修改或影响原来的鼠标悬停提示（CToolTipCtrl m_tool_tips）的任何行为。

class CPinTipWnd : public CWnd
{
    DECLARE_DYNAMIC(CPinTipWnd)

public:
    CPinTipWnd();
    virtual ~CPinTipWnd();

    // 创建固定提示窗口。pt为窗口首次显示时的建议位置（通常为鼠标当前坐标）。
    BOOL Create(CWnd* pParentWnd, CPoint pt);

    // 设置提示文本内容，会自动根据文本内容调整窗口大小。
    void SetTipText(const CString& text);

    // 在指定位置（默认为当前鼠标位置附近）显示固定提示窗口，并置于最顶层。
    // 如果窗口当前已经可见，则只会更新内容，不会移动窗口位置。
    void ShowPin();

    // 隐藏固定提示窗口（不会销毁窗口，可以随时再次调用ShowPin()显示）。
    void HidePin();

    // 固定提示窗口当前是否处于显示状态
    BOOL IsPinVisible() const;

protected:
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()

private:
    void UpdateWindowSize();
    void ClampToWorkArea(CRect& rect) const;
    CRect GetCloseButtonRect() const;

private:
    CString m_tip_text;        //当前显示的提示文本
    CFont m_font;               //绘制文字使用的字体
    BOOL m_font_created{ FALSE };
    static LPCTSTR CLASS_NAME;  //自定义窗口类名

    static const int PADDING = 8;          //文本与边框之间的内边距
    static const int HEADER_HEIGHT = 20;   //顶部预留给关闭按钮的高度
    static const int MAX_WIDTH = 320;      //提示窗口的最大宽度（不含内边距）
    static const int CLOSE_BTN_SIZE = 16;  //关闭按钮的大小
};
