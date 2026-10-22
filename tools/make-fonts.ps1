# Makes YouTubeCounter\fonts_big.h: big bold fonts for the 7" screen, in the
# same format as the fonts built into LovyanGFX (Adafruit GFX fonts).
#
#   .\tools\make-fonts.ps1 -Ttf C:\path\to\LiberationSans-Bold.ttf
#
# Liberation Sans is free to use and share (SIL Open Font License 1.1).
# Add -Preview to also save a picture of each font for checking.
param(
  [Parameter(Mandatory = $true)][string]$Ttf,
  [int[]]$Sizes = @(36, 48, 64, 84),
  [switch]$Preview
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$out = Join-Path $root 'YouTubeCounter\fonts_big.h'

Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Text;
using System.Text;

public static class GfxFontMaker {
  class Glyph { public int Offset, W, H, Adv, Xo, Yo; public bool[] Px; }

  // Renders printable ASCII at `pt` points (141 dpi, like Adafruit's fontconvert)
  public static string Make(string ttf, string name, int pt, string previewPath) {
    var pfc = new PrivateFontCollection();
    pfc.AddFontFile(ttf);
    var fam = pfc.Families[0];
    var style = fam.IsStyleAvailable(FontStyle.Bold) ? FontStyle.Bold : FontStyle.Regular;
    float em = pt * 141f / 72f;
    float ascent = fam.GetCellAscent(style) * em / fam.GetEmHeight(style);
    int yAdvance = (int)Math.Round(fam.GetLineSpacing(style) * em / fam.GetEmHeight(style));
    var fmt = (StringFormat)StringFormat.GenericTypographic.Clone();
    fmt.FormatFlags |= StringFormatFlags.MeasureTrailingSpaces;

    var glyphs = new List<Glyph>();
    var bytes = new List<byte>();
    using (var font = new Font(fam, em, style, GraphicsUnit.Pixel))
    using (var mb = new Bitmap(4, 4))
    using (var mg = Graphics.FromImage(mb)) {
      mg.TextRenderingHint = TextRenderingHint.AntiAlias;
      for (int c = 0x20; c <= 0x7E; c++) {
        string ch = ((char)c).ToString();
        var g = new Glyph();
        g.Offset = bytes.Count;
        g.Adv = (int)Math.Round(mg.MeasureString(ch, font, new PointF(0, 0), fmt).Width);
        using (var path = new GraphicsPath()) {
          path.AddString(ch, fam, (int)style, em, new PointF(0, 0), fmt);
          RectangleF b = path.GetBounds();
          if (b.Width > 0.5f && b.Height > 0.5f) {
            int x0 = (int)Math.Floor(b.X), y0 = (int)Math.Floor(b.Y);
            int x1 = (int)Math.Ceiling(b.Right), y1 = (int)Math.Ceiling(b.Bottom);
            g.W = x1 - x0; g.H = y1 - y0; g.Xo = x0; g.Yo = y0 - (int)Math.Round(ascent);
            g.Px = new bool[g.W * g.H];
            using (var bmp = new Bitmap(g.W, g.H))
            using (var gg = Graphics.FromImage(bmp)) {
              gg.Clear(Color.Black);
              gg.SmoothingMode = SmoothingMode.AntiAlias;
              gg.TranslateTransform(-x0, -y0);
              gg.FillPath(Brushes.White, path);
              int acc = 0, nbits = 0;
              for (int y = 0; y < g.H; y++)
                for (int x = 0; x < g.W; x++) {
                  bool on = bmp.GetPixel(x, y).R >= 128;
                  g.Px[y * g.W + x] = on;
                  acc = (acc << 1) | (on ? 1 : 0);
                  if (++nbits == 8) { bytes.Add((byte)acc); acc = 0; nbits = 0; }
                }
              if (nbits > 0) bytes.Add((byte)(acc << (8 - nbits)));
            }
          }
        }
        if (g.W > 255 || g.H > 255 || g.Adv > 255 || g.Xo < -128 || g.Xo > 127 || g.Yo < -128 || g.Yo > 127)
          throw new Exception(name + ": '" + ch + "' is too big for the font format");
        glyphs.Add(g);
      }
    }
    if (yAdvance > 255) throw new Exception(name + ": line height too big");

    if (previewPath != null) SavePreview(glyphs, yAdvance, ascent, previewPath);

    var sb = new StringBuilder();
    sb.AppendFormat("const uint8_t {0}Bitmaps[] PROGMEM = {{\n", name);
    for (int i = 0; i < bytes.Count; i++) {
      sb.AppendFormat("0x{0:X2}{1}", bytes[i], i + 1 < bytes.Count ? "," : "");
      if (i % 24 == 23) sb.Append("\n");
    }
    sb.Append("};\n");
    sb.AppendFormat("const lgfx::GFXglyph {0}Glyphs[] PROGMEM = {{\n", name);
    foreach (var g in glyphs)
      sb.AppendFormat("  {{{0},{1},{2},{3},{4},{5}}},\n", g.Offset, g.W, g.H, g.Adv, g.Xo, g.Yo);
    sb.Append("};\n");
    sb.AppendFormat("const lgfx::GFXfont {0} = {{(uint8_t*){0}Bitmaps, (lgfx::GFXglyph*){0}Glyphs, 0x20, 0x7E, {1}}};\n\n", name, yAdvance);
    return sb.ToString();
  }

  // Draws a test line using only the generated glyph data
  static void SavePreview(List<Glyph> glyphs, int yAdvance, float ascent, string path) {
    string text = "SubCounter 1,234,567 12:45 Any minute";
    int width = 20;
    foreach (char ch in text) width += glyphs[ch - 0x20].Adv;
    using (var bmp = new Bitmap(width, yAdvance + 20)) {
      using (var g = Graphics.FromImage(bmp)) g.Clear(Color.FromArgb(6, 10, 28));
      int pen = 10, baseline = 10 + (int)Math.Round(ascent);
      foreach (char ch in text) {
        var gl = glyphs[ch - 0x20];
        for (int y = 0; y < gl.H; y++)
          for (int x = 0; x < gl.W; x++)
            if (gl.Px[y * gl.W + x]) bmp.SetPixel(pen + gl.Xo + x, baseline + gl.Yo + y, Color.FromArgb(46, 230, 168));
        pen += gl.Adv;
      }
      bmp.Save(path);
    }
  }
}
'@

$header = @"
// Big bold fonts for the 7" screen, made by tools\make-fonts.ps1.
// Glyphs drawn from Liberation Sans Bold: digitized data copyright (c) 2010
// Google Corporation, copyright (c) 2012 Red Hat, Inc. Licensed under the
// SIL Open Font License 1.1 (scripts.sil.org/OFL).
#pragma once

"@
$names = @()
foreach ($pt in $Sizes) {
  $name = "BigSansBold$pt"
  $prev = $null
  if ($Preview) { $prev = Join-Path $env:TEMP "$name.png" }
  $header += [GfxFontMaker]::Make($Ttf, $name, $pt, $prev)
  $names += $name
  if ($prev) { Write-Output "Preview: $prev" }
}
[IO.File]::WriteAllText($out, $header)
Write-Output ("Wrote " + $out + " (" + ($names -join ', ') + ")")
