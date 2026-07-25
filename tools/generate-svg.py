#!/usr/bin/env python3
"""Regenerate sbb-face.svg with mathematically correct tick coordinates."""

import math

cx, cy = 52.5, 52.5

R_CIRCLE = 52.5
# Hour markers (every 5 minutes / 30 degrees)
HOUR_OUTER = 48.5
HOUR_INNER = 36.5
HOUR_WIDTH = 3.5

# Minute ticks (every minute except the 12 hour positions)
MIN_OUTER = 48.5
MIN_INNER = 45.0
MIN_WIDTH = 1.392


def tick_path(angle_deg, r_outer, r_inner, width):
    a = math.radians(angle_deg)
    rad = (math.sin(a), -math.cos(a))
    tan = (math.cos(a), math.sin(a))

    p1 = (cx + r_outer * rad[0] - width / 2 * tan[0],
          cy + r_outer * rad[1] - width / 2 * tan[1])
    p2 = (cx + r_outer * rad[0] + width / 2 * tan[0],
          cy + r_outer * rad[1] + width / 2 * tan[1])
    p3 = (cx + r_inner * rad[0] + width / 2 * tan[0],
          cy + r_inner * rad[1] + width / 2 * tan[1])
    p4 = (cx + r_inner * rad[0] - width / 2 * tan[0],
          cy + r_inner * rad[1] - width / 2 * tan[1])

    pts = [p1, p2, p3, p4]
    d = f"M{pts[0][0]:.3f} {pts[0][1]:.3f}"
    for i in range(1, 4):
        dx = pts[i][0] - pts[i - 1][0]
        dy = pts[i][1] - pts[i - 1][1]
        d += f"l{dx:+.3f} {dy:+.3f}"
    d += "z"
    return d


def generate_svg(face_fill, face_bg_fill):
    lines = []
    lines.append('<?xml version="1.0" encoding="UTF-8"?>')
    lines.append('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 105 105">')
    lines.append('  <defs>')
    lines.append('    <style>')
    lines.append(f'      .face {{ fill: {face_fill}; }}')
    lines.append(f'      .face-bg {{ fill: {face_bg_fill}; }}')
    lines.append('    </style>')
    lines.append('  </defs>')
    lines.append('')
    lines.append('  <!-- Clock face -->')
    lines.append('  <g class="face">')
    lines.append(f'    <circle class="face-bg" cx="{cx}" cy="{cy}" r="{R_CIRCLE}"/>')

    # Hour markers at every 5 minutes (0, 5, 10, ..., 55)
    hour_paths = []
    for h in range(12):
        angle = h * 30  # 0, 30, 60, ..., 330
        hour_paths.append(tick_path(angle, HOUR_OUTER, HOUR_INNER, HOUR_WIDTH))
    lines.append('    <path d="' + ''.join(hour_paths) + '"/>')

    # Minute/second ticks at every minute except multiples of 5
    min_paths = []
    for m in range(60):
        if m % 5 != 0:
            angle = m * 6
            min_paths.append(tick_path(angle, MIN_OUTER, MIN_INNER, MIN_WIDTH))
    lines.append('    <g>')
    lines.append('      <path d="' + ''.join(min_paths) + '"/>')
    lines.append('    </g>')
    lines.append('  </g>')
    lines.append('</svg>')
    return '\n'.join(lines) + '\n'


if __name__ == '__main__':
    import sys, re

    target = sys.argv[1] if len(sys.argv) > 1 else 'both'

    if target in ('white', 'both'):
        white_svg = generate_svg('#1a1a1a', '#fff')
        with open('sbb-face.svg', 'w') as f:
            f.write(white_svg)
        print('Generated sbb-face.svg')

    if target in ('black', 'both'):
        black_svg = generate_svg('#fff', '#000')
        with open('sbb-face-black.svg', 'w') as f:
            f.write(black_svg)
        print('Generated sbb-face-black.svg')
