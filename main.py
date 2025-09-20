import tkinter as tk
from tkinter import ttk, filedialog, messagebox, Text, Toplevel
import os
import sys
import shutil  # 用于检测命令是否存在


def create_desktop_launcher():
    # 创建主窗口
    root = tk.Tk()
    root.title("创建 .desktop 启动器")
    root.geometry("650x580")

    # 设置样式
    style = ttk.Style()
    style.theme_use('clam')

    # 主框架
    main_frame = ttk.Frame(root, padding="20")
    main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
    root.columnconfigure(0, weight=1)
    root.rowconfigure(0, weight=1)

    # Name 输入
    ttk.Label(main_frame, text="应用名称 (Name):").grid(row=0, column=0, sticky=tk.W, pady=(0, 5))
    name_entry = ttk.Entry(main_frame, width=50)
    name_entry.grid(row=1, column=0, sticky=(tk.W, tk.E), pady=(0, 15))

    # Exec 选项卡：单选按钮
    exec_var = tk.StringVar(value="file")
    ttk.Label(main_frame, text="执行方式:").grid(row=2, column=0, sticky=tk.W, pady=(0, 5))
    exec_frame = ttk.Frame(main_frame)
    exec_frame.grid(row=3, column=0, sticky=(tk.W, tk.E), pady=(0, 15))

    ttk.Radiobutton(exec_frame, text="选择可执行文件", variable=exec_var, value="file",
                    command=lambda: toggle_exec_input(True)).pack(side=tk.LEFT, padx=(0, 20))
    ttk.Radiobutton(exec_frame, text="手动输入命令", variable=exec_var, value="cmd",
                    command=lambda: toggle_exec_input(False)).pack(side=tk.LEFT)

    # 执行路径输入框
    exec_path_var = tk.StringVar()
    exec_path_entry = ttk.Entry(main_frame, textvariable=exec_path_var, width=50)
    exec_path_entry.grid(row=4, column=0, sticky=(tk.W, tk.E), pady=(0, 15))

    # 浏览按钮
    browse_btn = ttk.Button(main_frame, text="浏览...", command=lambda: browse_file(exec_path_var))
    browse_btn.grid(row=4, column=1, padx=(10, 0), sticky=tk.W)

    # Icon 选择
    ttk.Label(main_frame, text="图标文件 (Icon):").grid(row=5, column=0, sticky=tk.W, pady=(0, 5))
    icon_var = tk.StringVar()
    icon_entry = ttk.Entry(main_frame, textvariable=icon_var, width=50)
    icon_entry.grid(row=6, column=0, sticky=(tk.W, tk.E), pady=(0, 15))
    icon_browse_btn = ttk.Button(main_frame, text="浏览图标...", command=lambda: browse_icon(icon_var))
    icon_browse_btn.grid(row=6, column=1, padx=(10, 0), sticky=tk.W)

    # Terminal 勾选框
    terminal_var = tk.BooleanVar()
    terminal_check = ttk.Checkbutton(main_frame, text="在终端中运行 (Terminal=true)", variable=terminal_var)
    terminal_check.grid(row=7, column=0, sticky=tk.W, pady=(15, 15))

    # 目标位置选择
    ttk.Label(main_frame, text="保存位置:", font=("Arial", 10, "bold")).grid(row=8, column=0, sticky=tk.W, pady=(15, 5))
    location_var = tk.StringVar(value="desktop")  # 默认选桌面

    location_frame = ttk.Frame(main_frame)
    location_frame.grid(row=9, column=0, columnspan=2, sticky=tk.W, pady=(0, 15))

    ttk.Radiobutton(location_frame, text="保存到桌面 (~/桌面/)", variable=location_var, value="desktop").pack(anchor=tk.W)
    ttk.Radiobutton(location_frame, text="保存到应用菜单 (~/.local/share/applications/)", variable=location_var, value="apps").pack(anchor=tk.W)

    # 提示信息
    tip_label = ttk.Label(
        main_frame,
        text="💡 桌面：直接可见图标\n应用菜单：在启动器/应用程序列表中显示（需刷新）",
        foreground="#666",
        font=("Arial", 9)
    )
    tip_label.grid(row=10, column=0, columnspan=2, sticky=tk.W, pady=(0, 20))

    # 生成按钮
    def on_generate():
        name = name_entry.get().strip()
        exec_choice = exec_var.get()
        exec_value = exec_path_var.get().strip()
        icon_value = icon_var.get().strip()
        terminal = terminal_var.get()
        target = location_var.get()

        if not name:
            messagebox.showerror("错误", "应用名称不能为空！")
            return

        if exec_choice == "file":
            if not exec_value or not os.path.exists(exec_value):
                messagebox.showerror("错误", "请选择有效的可执行文件！")
                return
            exec_value = os.path.abspath(exec_value)
        elif exec_choice == "cmd":
            if not exec_value:
                messagebox.showerror("错误", "请输入命令！")
                return

        if icon_value and not os.path.exists(icon_value):
            messagebox.showwarning("警告", "图标文件不存在，将使用默认图标")

        # 确定目标目录
        if target == "desktop":
            desktop_dir = os.path.expanduser("~/桌面")
            if not os.path.exists(desktop_dir):
                try:
                    os.makedirs(desktop_dir)
                except Exception as e:
                    messagebox.showerror("错误", f"无法创建桌面目录: {e}")
                    return
            filepath = os.path.join(desktop_dir, f"{name}.desktop")
        else:  # target == "apps"
            apps_dir = os.path.expanduser("~/.local/share/applications")
            if not os.path.exists(apps_dir):
                try:
                    os.makedirs(apps_dir, exist_ok=True)
                except Exception as e:
                    messagebox.showerror("错误", f"无法创建应用目录: {e}")
                    return
            filepath = os.path.join(apps_dir, f"{name}.desktop")

        # 构建 .desktop 内容
        content = f"""[Desktop Entry]
Version=1.0
Type=Application
Name={name}
Exec={exec_value}
Icon={icon_value if icon_value else ""}
Terminal={'true' if terminal else 'false'}
Categories=Utility;
"""

        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            # 设置可执行权限（Linux/Mac）
            os.chmod(filepath, 0o755)

            if target == "desktop":
                msg = f".desktop 文件已生成至桌面：\n{filepath}\n\n双击即可运行。"
                messagebox.showinfo("成功", msg)
                root.quit()
            else:
                refresh_cmd = "update-desktop-database ~/.local/share/applications"

                # 检测命令是否存在
                has_cmd = shutil.which("update-desktop-database") is not None

                if has_cmd:
                    tip_text = "✅ 检测到 update-desktop-database 已安装，运行以下命令可立即刷新菜单："
                else:
                    tip_text = (
                        "⚠️ 未检测到 update-desktop-database，请先安装：\n"
                        "  Ubuntu/Debian:   sudo apt install desktop-file-utils\n"
                        "  Fedora/RHEL:     sudo dnf install desktop-file-utils\n"
                        "  Arch/Manjaro:    sudo pacman -S desktop-file-utils\n"
                        "安装后运行以下命令刷新菜单："
                    )

                # 创建轻量弹窗，聚焦“复制刷新命令”
                cmd_win = Toplevel(root)
                cmd_win.title("操作提示")
                cmd_win.geometry("650x320")
                cmd_win.resizable(False, False)

                msg = f".desktop 文件已生成至应用菜单：\n{filepath}"
                ttk.Label(cmd_win, text=msg, font=("Arial", 10), justify="left").pack(padx=20, pady=(15,5))
                ttk.Label(cmd_win, text=tip_text, font=("Arial", 9), foreground="#555", justify="left").pack(padx=20, pady=(5,5))

                # 命令显示框
                cmd_entry = ttk.Entry(cmd_win, font=("Courier", 10), width=75)
                cmd_entry.insert(0, refresh_cmd)
                cmd_entry.config(state="readonly")  # 不可编辑但可复制
                cmd_entry.pack(padx=20, pady=5)

                # 复制按钮
                def copy_cmd():
                    cmd_win.clipboard_clear()
                    cmd_win.clipboard_append(refresh_cmd)
                    messagebox.showinfo("复制成功", "命令已复制到剪贴板！", parent=cmd_win)

                copy_btn = ttk.Button(cmd_win, text="📋 复制命令", command=copy_cmd)
                copy_btn.pack(pady=10)

                # 关闭按钮
                close_btn = ttk.Button(cmd_win, text="关闭", command=lambda: [cmd_win.destroy(), root.quit()])
                close_btn.pack(pady=5)

                cmd_win.transient(root)
                cmd_win.grab_set()
                root.wait_window(cmd_win)

        except Exception as e:
            messagebox.showerror("错误", f"写入文件失败：{e}")

    generate_btn = ttk.Button(main_frame, text="生成 .desktop 文件", command=on_generate)
    generate_btn.grid(row=11, column=0, columnspan=2, pady=(30, 0))

    # 辅助函数
    def browse_file(var):
        path = filedialog.askopenfilename(
            title="选择可执行文件",
            filetypes=[
                ("所有可执行文件", "*"),
                ("可执行文件", "*.exe *.bin *.sh *.out"),
                ("Shell 脚本", "*.sh"),
                ("Python 脚本", "*.py")
            ]
        )
        if path:
            var.set(path)

    def browse_icon(var):
        path = filedialog.askopenfilename(
            title="选择图标文件",
            filetypes=[
                ("图标文件", "*.png *.ico *.svg *.jpg *.jpeg *.bmp"),
                ("PNG 图像", "*.png"),
                ("ICO 图标", "*.ico"),
                ("SVG 图像", "*.svg"),
                ("JPEG 图像", "*.jpg *.jpeg"),
                ("BMP 图像", "*.bmp"),
                ("所有文件", "*.*")
            ]
        )
        if path:
            var.set(path)

    def toggle_exec_input(is_file_mode):
        if is_file_mode:
            exec_path_entry.config(state='normal')
            browse_btn.config(state='normal')
        else:
            exec_path_entry.config(state='normal')
            browse_btn.config(state='disabled')

    # 初始化状态
    toggle_exec_input(True)  # 默认选择“选择可执行文件”

    # 运行主循环
    root.mainloop()


if __name__ == "__main__":
    create_desktop_launcher()