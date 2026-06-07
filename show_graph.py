import os
import sys
import json
import math
import tkinter as tk
from tkinter import messagebox

# ---------------------------------------------------------
# Theme Constants (Premium Sleek Dark Mode)
# ---------------------------------------------------------
COLOR_BG = "#121212"          # Very dark grey
COLOR_CARD = "#1E1E1E"        # Lighter card container
COLOR_BORDER = "#2D2D2D"      # Subtle border/gridline
COLOR_TEXT_PRIMARY = "#FFFFFF"# High contrast white
COLOR_TEXT_MUTED = "#8E8E93"  # Muted grey text
COLOR_ACCENT = "#00E5FF"      # Electric Cyan

# Custom harmonious neon colors for charts
COLORS_PALETTE = [
    "#00E676",  # Neon Green (Success / Completed / Active)
    "#00E5FF",  # Neon Cyan (Secondary Info / E-Wallet)
    "#D500F9",  # Neon Purple (Photographers / Credit Card)
    "#FFD600",  # Neon Yellow (Pending / Online Banking)
    "#FF1744",  # Neon Red (Rejected / Banned / Disputed)
    "#FF9100",  # Neon Orange
    "#2979FF"   # Neon Blue
]

STATUS_COLORS = {
    "Completed": "#00E676",
    "Approved": "#00E5FF",
    "Deposit Paid": "#2979FF",
    "Pending": "#FFD600",
    "Rejected": "#FF1744"
}

# ---------------------------------------------------------
# Helper Functions
# ---------------------------------------------------------
def load_data():
    """Loads graph data from local JSON file."""
    filepath = "graph_data.json"
    if not os.path.exists(filepath):
        return None
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading JSON: {e}")
        return None

# ---------------------------------------------------------
# Tooltip/Hover Info class
# ---------------------------------------------------------
class ChartHoverPanel:
    def __init__(self, parent_frame):
        self.frame = tk.Frame(parent_frame, bg=COLOR_CARD, bd=1, relief="flat", highlightbackground=COLOR_BORDER, highlightthickness=1)
        self.frame.pack(side="bottom", fill="x", padx=20, pady=(10, 20))
        
        self.label_title = tk.Label(self.frame, text="HOVER DATA POINT TO EXPLORE", font=("Arial", 10, "bold"), fg=COLOR_ACCENT, bg=COLOR_CARD)
        self.label_title.pack(anchor="w", padx=15, pady=(8, 2))
        
        self.label_value = tk.Label(self.frame, text="Move your mouse cursor over elements in the graph to see live details.", font=("Arial", 9), fg=COLOR_TEXT_MUTED, bg=COLOR_CARD)
        self.label_value.pack(anchor="w", padx=15, pady=(2, 8))

    def update(self, title, value):
        self.label_title.config(text=title.upper())
        self.label_value.config(text=value, fg=COLOR_TEXT_PRIMARY)

    def reset(self):
        self.label_title.config(text="HOVER DATA POINT TO EXPLORE")
        self.label_value.config(text="Move your mouse cursor over elements in the graph to see live details.", fg=COLOR_TEXT_MUTED)


# ---------------------------------------------------------
# Modern Side Menu Bar
# ---------------------------------------------------------
class SideMenu(tk.Frame):
    def __init__(self, parent, change_tab_callback):
        super().__init__(parent, bg=COLOR_CARD, width=220)
        self.pack_propagate(False)
        self.pack(side="left", fill="y")
        
        # Header/Brand
        brand_frame = tk.Frame(self, bg=COLOR_CARD)
        brand_frame.pack(fill="x", pady=(25, 30))
        
        tk.Label(brand_frame, text="PHOTOGRAPHY", font=("Arial", 12, "bold"), fg=COLOR_ACCENT, bg=COLOR_CARD).pack()
        tk.Label(brand_frame, text="Admin Analytics", font=("Arial", 9), fg=COLOR_TEXT_MUTED, bg=COLOR_CARD).pack()
        
        # Separator line
        sep = tk.Frame(self, bg=COLOR_BORDER, height=1)
        sep.pack(fill="x", padx=15, pady=(0, 20))
        
        # Tab Buttons
        self.buttons = []
        self.tabs = [
            ("Photographers", "📊  Performers"),
            ("Revenue", "📈  Revenue Trend"),
            ("Statuses", "📥  Booking Status"),
            ("Payments", "💳  Payment Mix")
        ]
        
        self.active_tab = 0
        for i, (tab_id, label) in enumerate(self.tabs):
            btn = tk.Button(
                self, 
                text=label, 
                font=("Arial", 10), 
                fg=COLOR_TEXT_MUTED, 
                bg=COLOR_CARD,
                activeforeground=COLOR_TEXT_PRIMARY,
                activebackground=COLOR_BG,
                bd=0, 
                relief="flat",
                anchor="w", 
                padx=20,
                pady=12,
                cursor="hand2",
                command=lambda idx=i, cb=change_tab_callback: self.select_tab(idx, cb)
            )
            btn.pack(fill="x", padx=10, pady=2)
            # Bind hover animations
            btn.bind("<Enter>", lambda e, b=btn: self.on_hover(b))
            btn.bind("<Leave>", lambda e, b=btn, idx=i: self.on_leave(b, idx))
            self.buttons.append(btn)
            
        self.update_button_styles()

        # Footer version
        tk.Label(self, text="v1.2.0 (Stable)", font=("Arial", 8), fg=COLOR_TEXT_MUTED, bg=COLOR_CARD).pack(side="bottom", pady=15)

    def select_tab(self, index, callback):
        self.active_tab = index
        self.update_button_styles()
        callback(self.tabs[index][0])

    def on_hover(self, btn):
        if self.buttons.index(btn) != self.active_tab:
            btn.config(bg="#27272A", fg=COLOR_TEXT_PRIMARY)

    def on_leave(self, btn, idx):
        if idx != self.active_tab:
            btn.config(bg=COLOR_CARD, fg=COLOR_TEXT_MUTED)

    def update_button_styles(self):
        for i, btn in enumerate(self.buttons):
            if i == self.active_tab:
                btn.config(bg=COLOR_BG, fg=COLOR_ACCENT, font=("Arial", 10, "bold"))
            else:
                btn.config(bg=COLOR_CARD, fg=COLOR_TEXT_MUTED, font=("Arial", 10))

# ---------------------------------------------------------
# Main Graph Application
# ---------------------------------------------------------
class GraphApp(tk.Tk):
    def __init__(self, data):
        super().__init__()
        self.title("Photography Booking System - Admin Reports Graph")
        self.geometry("960x650")
        self.configure(bg=COLOR_BG)
        self.resizable(False, False)
        
        self.data = data
        
        # Center Window on Screen
        self.update_idletasks()
        w, h = 960, 650
        sx = (self.winfo_screenwidth() - w) // 2
        sy = (self.winfo_screenheight() - h) // 2
        self.geometry(f"{w}x{h}+{sx}+{sy}")
        
        # Build Side Menu
        self.sidebar = SideMenu(self, self.switch_tab)
        
        # Main Display Panel
        self.main_panel = tk.Frame(self, bg=COLOR_BG)
        self.main_panel.pack(side="right", fill="both", expand=True)
        
        # Tab Header Frame
        self.header_frame = tk.Frame(self.main_panel, bg=COLOR_BG)
        self.header_frame.pack(fill="x", padx=20, pady=(20, 10))
        
        self.label_tab_title = tk.Label(self.header_frame, text="Photographer Performance", font=("Arial", 16, "bold"), fg=COLOR_TEXT_PRIMARY, bg=COLOR_BG)
        self.label_tab_title.pack(anchor="w")
        
        self.label_tab_desc = tk.Label(self.header_frame, text="Overview of bookings completed and revenue generated per active freelancer.", font=("Arial", 9), fg=COLOR_TEXT_MUTED, bg=COLOR_BG)
        self.label_tab_desc.pack(anchor="w", pady=(2, 0))
        
        # Graph Canvas Container
        self.canvas_frame = tk.Frame(self.main_panel, bg=COLOR_CARD, bd=1, relief="flat", highlightbackground=COLOR_BORDER, highlightthickness=1)
        self.canvas_frame.pack(fill="both", expand=True, padx=20, pady=10)
        
        self.canvas = tk.Canvas(self.canvas_frame, bg=COLOR_CARD, bd=0, highlightthickness=0)
        self.canvas.pack(fill="both", expand=True, padx=10, pady=10)
        
        # Detail Hover Panel
        self.hover_panel = ChartHoverPanel(self.main_panel)
        
        # Initial Render
        self.current_tab = "Photographers"
        self.canvas.bind("<Configure>", lambda e: self.render_current_tab())

    def switch_tab(self, tab_id):
        self.current_tab = tab_id
        if tab_id == "Photographers":
            self.label_tab_title.config(text="Photographer Performance")
            self.label_tab_desc.config(text="Rankings of active photographers by bookings completed and total RM earned.")
        elif tab_id == "Revenue":
            self.label_tab_title.config(text="Revenue Trend Over Time")
            self.label_tab_desc.config(text="Cumulative booking payment trend. Demonstrates gross growth over active dates.")
        elif tab_id == "Statuses":
            self.label_tab_title.config(text="Booking Status Distribution")
            self.label_tab_desc.config(text="Breakdown of jobs by their operational lifecycle phase (Pending vs Completed).")
        elif tab_id == "Payments":
            self.label_tab_title.config(text="Payment Methods Split")
            self.label_tab_desc.config(text="Analytics demonstrating the utilization rate of payment channels (E-Wallet, Card, Banking).")
            
        self.hover_panel.reset()
        self.render_current_tab()

    def render_current_tab(self):
        self.canvas.delete("all")
        self.canvas.unbind("<Motion>")
        
        if not self.data:
            self.draw_empty_state("No data loaded. Run queries in C++ dashboard first.")
            return
            
        if self.current_tab == "Photographers":
            self.render_photographers()
        elif self.current_tab == "Revenue":
            self.render_revenue()
        elif self.current_tab == "Statuses":
            self.render_statuses()
        elif self.current_tab == "Payments":
            self.render_payments()

    def draw_empty_state(self, message):
        cw = self.canvas.winfo_width()
        ch = self.canvas.winfo_height()
        if cw < 100: cw = 680
        if ch < 100: ch = 350
        
        self.canvas.create_text(
            cw // 2, ch // 2, 
            text=message, 
            font=("Arial", 11), 
            fill=COLOR_TEXT_MUTED, 
            justify="center"
        )

    # ---------------------------------------------------------
    # TAB 1: PHOTOGRAPHER PERFORMANCE (BAR CHART)
    # ---------------------------------------------------------
    def render_photographers(self):
        ph_list = self.data.get("photographers", [])
        if not ph_list:
            self.draw_empty_state("No photographer performance metrics recorded in DB.")
            return
            
        cw = self.canvas.winfo_width()
        ch = self.canvas.winfo_height()
        if cw < 100: cw = 680
        if ch < 100: ch = 350
        
        # Grid boundaries
        pad_l, pad_r, pad_t, pad_b = 60, 40, 40, 50
        gw = cw - pad_l - pad_r
        gh = ch - pad_t - pad_b
        
        # Get Max Earnings to scale Y-axis
        max_earned = max([ph.get("earned", 0) for ph in ph_list] + [1000])
        # Round up max_earned to neat units (e.g. multiples of 1000 or 500)
        scale_max = math.ceil(max_earned / 1000) * 1000 if max_earned > 1000 else 1000
        
        # Draw Y-Axis Gridlines
        grid_count = 5
        for i in range(grid_count + 1):
            y_val = i * (scale_max / grid_count)
            gy = pad_t + gh - int((y_val / scale_max) * gh)
            self.canvas.create_line(pad_l, gy, cw - pad_r, gy, fill=COLOR_BORDER, dash=(2, 2))
            self.canvas.create_text(pad_l - 10, gy, text=f"RM {int(y_val)}", font=("Arial", 8), fill=COLOR_TEXT_MUTED, anchor="e")

        # Draw X-Axis Line
        self.canvas.create_line(pad_l, pad_t + gh, cw - pad_r, pad_t + gh, fill=COLOR_BORDER, width=2)
        
        # Draw Bars
        n = len(ph_list)
        bar_width = min(45, int((gw / n) * 0.5))
        spacing = int(gw / n)
        
        bars = []
        for i, ph in enumerate(ph_list):
            name = ph.get("name", "Unknown")
            earned = ph.get("earned", 0)
            jobs = ph.get("jobs", 0)
            
            # Position
            bx = pad_l + int((i + 0.5) * spacing)
            by_bottom = pad_t + gh
            bar_h = int((earned / scale_max) * gh) if scale_max > 0 else 0
            by_top = by_bottom - bar_h
            
            # Draw bar rect
            color_index = i % len(COLORS_PALETTE)
            bar_color = COLORS_PALETTE[color_index]
            
            rect_id = self.canvas.create_rectangle(
                bx - bar_width//2, by_top,
                bx + bar_width//2, by_bottom,
                fill=bar_color, 
                outline=COLOR_BORDER,
                width=1,
                activefill=COLOR_TEXT_PRIMARY  # Highlight on hover
            )
            
            # Draw visual badge representing Jobs Completed inside/on top of bar
            text_color = COLOR_TEXT_PRIMARY if bar_h < 25 else COLOR_BG
            self.canvas.create_text(
                bx, by_top - 12 if bar_h < 20 else by_top + 10, 
                text=f"{jobs} jobs", 
                font=("Arial", 8, "bold"), 
                fill=COLOR_ACCENT if bar_h < 20 else COLOR_BG
            )
            
            # Label
            truncated_name = name if len(name) <= 12 else name[:10] + "..."
            self.canvas.create_text(
                bx, by_bottom + 15, 
                text=truncated_name, 
                font=("Arial", 8, "bold"), 
                fill=COLOR_TEXT_PRIMARY, 
                angle=15, 
                anchor="n"
            )
            
            # Store structural details for hover events
            bars.append({
                "id": rect_id,
                "name": name,
                "earned": earned,
                "jobs": jobs,
                "color": bar_color
            })
            
        # Hover handling
        def on_mouse_move(event):
            mx, my = event.x, event.y
            hovered_any = False
            for b in bars:
                coords = self.canvas.coords(b["id"])
                if coords and (coords[0] <= mx <= coords[2]) and (coords[1] <= my <= coords[3]):
                    # Highlight this photographer!
                    self.hover_panel.update(
                        f"Photographer Performance: {b['name']}",
                        f"Freelancer has successfully delivered {b['jobs']} booking assignments. Total Gross Revenue Generated: RM {b['earned']:.2f}."
                    )
                    hovered_any = True
                    break
            if not hovered_any:
                self.hover_panel.reset()
                
        self.canvas.bind("<Motion>", on_mouse_move)

    # ---------------------------------------------------------
    # TAB 2: REVENUE TREND OVER TIME (LINE CHART)
    # ---------------------------------------------------------
    def render_revenue(self):
        trend = self.data.get("revenue_trend", [])
        if not trend:
            self.draw_empty_state("No completed transactions recorded to plot revenue growth.")
            return
            
        # Chronological sorting just in case
        trend_sorted = sorted(trend, key=lambda x: x.get("date", ""))
        
        # Calculate cumulative revenue
        dates = []
        cumulative_rev = []
        curr_total = 0.0
        
        for t in trend_sorted:
            d = t.get("date", "Unknown")
            amt = t.get("amount", 0.0)
            curr_total += amt
            dates.append(d)
            cumulative_rev.append(curr_total)
            
        cw = self.canvas.winfo_width()
        ch = self.canvas.winfo_height()
        if cw < 100: cw = 680
        if ch < 100: ch = 350
        
        pad_l, pad_r, pad_t, pad_b = 60, 50, 40, 50
        gw = cw - pad_l - pad_r
        gh = ch - pad_t - pad_b
        
        # Get scale parameters
        max_y = cumulative_rev[-1] if cumulative_rev else 1000
        scale_max = math.ceil(max_y / 1000) * 1000 if max_y > 1000 else 1000
        
        # Draw Y-Axis Gridlines & Labels
        grid_count = 5
        for i in range(grid_count + 1):
            y_val = i * (scale_max / grid_count)
            gy = pad_t + gh - int((y_val / scale_max) * gh)
            self.canvas.create_line(pad_l, gy, cw - pad_r, gy, fill=COLOR_BORDER, dash=(2, 2))
            self.canvas.create_text(pad_l - 10, gy, text=f"RM {int(y_val)}", font=("Arial", 8), fill=COLOR_TEXT_MUTED, anchor="e")

        # Draw Axis Lines
        self.canvas.create_line(pad_l, pad_t + gh, cw - pad_r, pad_t + gh, fill=COLOR_BORDER, width=2)
        self.canvas.create_line(pad_l, pad_t, pad_l, pad_t + gh, fill=COLOR_BORDER, width=1)
        
        # Calculate points
        n = len(dates)
        spacing = int(gw / (n - 1)) if n > 1 else gw
        
        points = []
        for i in range(n):
            px = pad_l + (i * spacing)
            val_y = cumulative_rev[i]
            py = pad_t + gh - int((val_y / scale_max) * gh) if scale_max > 0 else pad_t + gh
            points.append((px, py, dates[i], val_y))
            
            # Format and draw Date labels for subset to avoid overlaps
            if n <= 8 or i % (n // 4 + 1) == 0 or i == n - 1:
                # Format to short date
                disp_date = dates[i]
                if len(disp_date) >= 10:
                    disp_date = disp_date[5:10] # MM-DD format
                self.canvas.create_text(px, pad_t + gh + 15, text=disp_date, font=("Arial", 8), fill=COLOR_TEXT_MUTED)

        # Plot Trend Line (Neon Green)
        for i in range(n - 1):
            self.canvas.create_line(
                points[i][0], points[i][1], 
                points[i+1][0], points[i+1][1], 
                fill="#00E676", 
                width=3
            )
            
        # Draw Data Points (interactive circles)
        circle_items = []
        for i, (px, py, date_str, val) in enumerate(points):
            pt_id = self.canvas.create_oval(
                px - 6, py - 6, 
                px + 6, py + 6, 
                fill=COLOR_CARD, 
                outline="#00E676", 
                width=2
            )
            circle_items.append({
                "id": pt_id,
                "x": px,
                "y": py,
                "date": date_str,
                "value": val
            })
            
        # Hover handling
        def on_mouse_move(event):
            mx, my = event.x, event.y
            hovered_any = False
            for pt in circle_items:
                dist = math.sqrt((pt["x"] - mx)**2 + (pt["y"] - my)**2)
                if dist <= 10:
                    # Glow point
                    self.canvas.itemconfig(pt["id"], fill="#00E676")
                    self.hover_panel.update(
                        f"Cumulative Revenue Trend: {pt['date']}",
                        f"Total system booking payments accumulated up to this date: RM {pt['value']:.2f}. Admin Commission (5%): RM {(pt['value'] * 0.05):.2f}."
                    )
                    hovered_any = True
                else:
                    self.canvas.itemconfig(pt["id"], fill=COLOR_CARD)
            if not hovered_any:
                self.hover_panel.reset()
                
        self.canvas.bind("<Motion>", on_mouse_move)

    # ---------------------------------------------------------
    # TAB 3: BOOKING STATUS DISTRIBUTION (PIE CHART)
    # ---------------------------------------------------------
    def render_statuses(self):
        statuses = self.data.get("booking_statuses", {})
        if not statuses or sum(statuses.values()) == 0:
            self.draw_empty_state("No booking operational statuses recorded in DB.")
            return
            
        total_bookings = sum(statuses.values())
        
        cw = self.canvas.winfo_width()
        ch = self.canvas.winfo_height()
        if cw < 100: cw = 680
        if ch < 100: ch = 350
        
        # Layout metrics
        px = 220
        py = ch // 2
        r = min(120, ch // 2 - 40)
        
        start_angle = 90
        slices = []
        
        for status, val in statuses.items():
            if val == 0:
                continue
            percent = (val / total_bookings) * 100
            angle_extent = (val / total_bookings) * 360.0
            
            # Map specific status colors
            slice_color = STATUS_COLORS.get(status, "#7F7F7F")
            
            # Create interactive arc slice
            arc_id = self.canvas.create_arc(
                px - r, py - r,
                px + r, py + r,
                start=start_angle,
                extent=angle_extent,
                fill=slice_color,
                outline=COLOR_BG,
                width=2
            )
            
            slices.append({
                "id": arc_id,
                "status": status,
                "count": val,
                "percent": percent,
                "start": start_angle,
                "extent": angle_extent,
                "color": slice_color
            })
            
            start_angle += angle_extent

        # Draw Side Legend
        legend_start_y = py - (len(slices) * 15)
        for i, sl in enumerate(slices):
            ly = legend_start_y + (i * 30)
            lx = px + r + 80
            
            # Colored Bullet
            self.canvas.create_rectangle(
                lx, ly - 6, lx + 12, ly + 6, 
                fill=sl["color"], 
                outline=""
            )
            
            # Text
            lbl = f"{sl['status']} ({sl['count']} bookings)"
            self.canvas.create_text(
                lx + 22, ly, 
                text=lbl, 
                font=("Arial", 9, "bold"), 
                fill=COLOR_TEXT_PRIMARY, 
                anchor="w"
            )
            self.canvas.create_text(
                lx + 22, ly + 12, 
                text=f"{sl['percent']:.1f}% share of operational pipeline", 
                font=("Arial", 8), 
                fill=COLOR_TEXT_MUTED, 
                anchor="w"
            )

        # Center indicator text
        self.canvas.create_text(
            px + r + 80, legend_start_y - 35,
            text=f"TOTAL BOOKINGS: {total_bookings}",
            font=("Arial", 11, "bold"),
            fill=COLOR_ACCENT,
            anchor="w"
        )
        
        # Hover handling using math-based geometry (angles and distances)
        def on_mouse_move(event):
            mx, my = event.x, event.y
            dx = mx - px
            dy = my - py
            dist = math.sqrt(dx**2 + dy**2)
            
            hovered_any = False
            if dist <= r:
                # Calculate angle of cursor in degrees [0, 360) clockwise starting from top/Y-axis
                cursor_angle = math.degrees(math.atan2(-dy, dx))
                # Adjust to math angles matching tkinter: 0 is East, 90 is North
                # Tkinter arcs start from angle and extend counter-clockwise.
                if cursor_angle < 0:
                    cursor_angle += 360.0
                
                # Convert standard math angle to matches tkinter arc angle mapping
                # Tkinter: 90 deg is Top/North.
                tk_angle = cursor_angle
                
                for sl in slices:
                    # Check if angle falls within slice domain
                    end_ang = (sl["start"] + sl["extent"]) % 360
                    # Handle modular overlap cases
                    angle_in_domain = False
                    test_angle = tk_angle
                    
                    # Normalize angles to [0, 360)
                    start = sl["start"] % 360
                    extent = sl["extent"]
                    
                    # Compute relative delta
                    diff = (test_angle - start) % 360
                    if diff <= extent:
                        angle_in_domain = True
                        
                    if angle_in_domain:
                        self.canvas.itemconfig(sl["id"], outline=COLOR_TEXT_PRIMARY, width=3)
                        self.hover_panel.update(
                            f"Booking Status: {sl['status']}",
                            f"{sl['count']} jobs are currently in this phase. Comprises {sl['percent']:.1f}% of total booking workload across the freelance system."
                        )
                        hovered_any = True
                    else:
                        self.canvas.itemconfig(sl["id"], outline=COLOR_BG, width=2)
            else:
                for sl in slices:
                    self.canvas.itemconfig(sl["id"], outline=COLOR_BG, width=2)
                    
            if not hovered_any:
                self.hover_panel.reset()
                
        self.canvas.bind("<Motion>", on_mouse_move)

    # ---------------------------------------------------------
    # TAB 4: PAYMENT METHODS (DONUT CHART)
    # ---------------------------------------------------------
    def render_payments(self):
        payments = self.data.get("payment_methods", {})
        if not payments or sum(payments.values()) == 0:
            self.draw_empty_state("No payment transaction records in DB to evaluate channels.")
            return
            
        total_payments = sum(payments.values())
        
        cw = self.canvas.winfo_width()
        ch = self.canvas.winfo_height()
        if cw < 100: cw = 680
        if ch < 100: ch = 350
        
        px = 220
        py = ch // 2
        r = min(120, ch // 2 - 40)
        
        start_angle = 90
        slices = []
        
        for i, (method, val) in enumerate(payments.items()):
            if val == 0:
                continue
            percent = (val / total_payments) * 100
            angle_extent = (val / total_payments) * 360.0
            
            slice_color = COLORS_PALETTE[i % len(COLORS_PALETTE)]
            
            arc_id = self.canvas.create_arc(
                px - r, py - r,
                px + r, py + r,
                start=start_angle,
                extent=angle_extent,
                fill=slice_color,
                outline=COLOR_BG,
                width=2
            )
            
            slices.append({
                "id": arc_id,
                "method": method,
                "count": val,
                "percent": percent,
                "start": start_angle,
                "extent": angle_extent,
                "color": slice_color
            })
            
            start_angle += angle_extent

        # Draw a inner circle to turn it into a gorgeous modern Donut chart
        inner_r = int(r * 0.55)
        self.canvas.create_oval(
            px - inner_r, py - inner_r,
            px + inner_r, py + inner_r,
            fill=COLOR_CARD,
            outline=COLOR_BORDER,
            width=1
        )
        
        # Center text inside the donut
        self.canvas.create_text(
            px, py - 8,
            text="MIX",
            font=("Arial", 9, "bold"),
            fill=COLOR_TEXT_MUTED
        )
        self.canvas.create_text(
            px, py + 8,
            text=f"{total_payments} Tx",
            font=("Arial", 11, "bold"),
            fill=COLOR_TEXT_PRIMARY
        )

        # Draw Side Legend
        legend_start_y = py - (len(slices) * 15)
        for i, sl in enumerate(slices):
            ly = legend_start_y + (i * 30)
            lx = px + r + 80
            
            # Colored Bullet
            self.canvas.create_rectangle(
                lx, ly - 6, lx + 12, ly + 6, 
                fill=sl["color"], 
                outline=""
            )
            
            # Text
            lbl = f"{sl['method']} ({sl['count']} Tx)"
            self.canvas.create_text(
                lx + 22, ly, 
                text=lbl, 
                font=("Arial", 9, "bold"), 
                fill=COLOR_TEXT_PRIMARY, 
                anchor="w"
            )
            self.canvas.create_text(
                lx + 22, ly + 12, 
                text=f"{sl['percent']:.1f}% of booking transactions", 
                font=("Arial", 8), 
                fill=COLOR_TEXT_MUTED, 
                anchor="w"
            )

        # Title for side panel
        self.canvas.create_text(
            px + r + 80, legend_start_y - 35,
            text="CHANNEL DISTRIBUTION",
            font=("Arial", 11, "bold"),
            fill=COLOR_ACCENT,
            anchor="w"
        )
        
        # Hover handling using math-based geometry (donut specific, ignoring the inner hollow radius)
        def on_mouse_move(event):
            mx, my = event.x, event.y
            dx = mx - px
            dy = my - py
            dist = math.sqrt(dx**2 + dy**2)
            
            hovered_any = False
            # Ensure it is in the donut ring (outer radius <= r and inner radius >= inner_r)
            if inner_r <= dist <= r:
                cursor_angle = math.degrees(math.atan2(-dy, dx))
                if cursor_angle < 0:
                    cursor_angle += 360.0
                
                tk_angle = cursor_angle
                
                for sl in slices:
                    start = sl["start"] % 360
                    extent = sl["extent"]
                    
                    diff = (tk_angle - start) % 360
                    if diff <= extent:
                        self.canvas.itemconfig(sl["id"], outline=COLOR_TEXT_PRIMARY, width=3)
                        self.hover_panel.update(
                            f"Payment Method: {sl['method']}",
                            f"Customers selected this channel for {sl['count']} transactions. Accounting for {sl['percent']:.1f}% of payment volume processed."
                        )
                        hovered_any = True
                    else:
                        self.canvas.itemconfig(sl["id"], outline=COLOR_BG, width=2)
            else:
                for sl in slices:
                    self.canvas.itemconfig(sl["id"], outline=COLOR_BG, width=2)
                    
            if not hovered_any:
                self.hover_panel.reset()
                
        self.canvas.bind("<Motion>", on_mouse_move)

# ---------------------------------------------------------
# Application Entry Point
# ---------------------------------------------------------
if __name__ == "__main__":
    # Load JSON data
    data = load_data()
    
    # Initialize UI
    app = GraphApp(data)
    app.mainloop()
