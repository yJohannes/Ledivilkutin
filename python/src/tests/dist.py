
import numpy as np
import matplotlib.pyplot as plt

SIZE = 256

# Coordinate grid
x = np.arange(SIZE)
y = np.arange(SIZE)
X, Y = np.meshgrid(x, y)

# Formulas
L_r = np.sqrt(X**2 + Y**2)

L_a = (
    np.maximum(np.abs(X), np.abs(Y)) / 2
    + np.minimum(np.abs(X), np.abs(Y))
)

D = L_r - L_a


# Signed logarithmic gradient
# Small differences become much more visible.

scale = 0.5  # Lower = stronger contrast near zero

N = np.sign(D) * np.log1p(np.abs(D) / scale)

# Normalize to [-1, 1]
N /= np.log1p(np.max(np.abs(D)) / scale)

# Create RGB gradient
colors = np.zeros((SIZE, SIZE, 3))

# Negative -> red, positive -> blue
colors[:, :, 0] = np.maximum(-N, 0)
colors[:, :, 2] = np.maximum(N, 0)


# Initial point
point_x, point_y = 128, 128

fig, ax = plt.subplots(figsize=(9, 9))

ax.imshow(
    colors,
    origin="lower",
    extent=[0, 255, 0, 255],
    interpolation="nearest"
)

point, = ax.plot(
    [], [], "yo",
    markersize=9,
    markeredgecolor="black"
)

info = ax.text(
    5, 250, "",
    color="white",
    fontsize=11,
    verticalalignment="top",
    bbox=dict(facecolor="black", alpha=0.8)
)


def update_point(px, py):
    global point_x, point_y

    point_x = int(np.clip(px, 0, 255))
    point_y = int(np.clip(py, 0, 255))

    lr = np.sqrt(point_x**2 + point_y**2)

    la = (
        max(abs(point_x), abs(point_y)) / 1
        + min(abs(point_x), abs(point_y)) / 2
    )

    difference = lr - la

    point.set_data([point_x], [point_y])

    info.set_text(
        f"x0 = {point_x}, y0 = {point_y}\n"
        f"Lr = {lr:.4f}\n"
        f"La = {la:.4f}\n"
        f"Lr - La = {difference:.4f}"
    )

    fig.canvas.draw_idle()


def on_mouse(event):
    if event.inaxes != ax:
        return

    if event.xdata is None or event.ydata is None:
        return

    update_point(event.xdata, event.ydata)


fig.canvas.mpl_connect("button_press_event", on_mouse)
fig.canvas.mpl_connect("motion_notify_event", on_mouse)

update_point(point_x, point_y)

ax.set_xlim(0, 255)
ax.set_ylim(0, 255)
ax.set_xlabel("x")
ax.set_ylabel("y")
ax.set_title("Gradient of Lr - La")

plt.tight_layout()
plt.show()
