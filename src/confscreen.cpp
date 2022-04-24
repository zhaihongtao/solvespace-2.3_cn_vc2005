//-----------------------------------------------------------------------------
// For the configuration screen, setup items that are not specific to the
// file being edited right now.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

void TextWindow::ScreenChangeLightDirection(int link, uint32_t v) {
    SS.TW.ShowEditControl(8, ssprintf("%.2f, %.2f, %.2f", CO(SS.lightDir[v])));
    SS.TW.edit.meaning = EDIT_LIGHT_DIRECTION;
    SS.TW.edit.i = v;
}

void TextWindow::ScreenChangeLightIntensity(int link, uint32_t v) {
    SS.TW.ShowEditControl(34, ssprintf("%.2f", SS.lightIntensity[v]));
    SS.TW.edit.meaning = EDIT_LIGHT_INTENSITY;
    SS.TW.edit.i = v;
}

void TextWindow::ScreenChangeColor(int link, uint32_t v) {
    SS.TW.ShowEditControlWithColorPicker(13, SS.modelColor[v]);

    SS.TW.edit.meaning = EDIT_COLOR;
    SS.TW.edit.i = v;
}

void TextWindow::ScreenChangeChordTolerance(int link, uint32_t v) {
    SS.TW.ShowEditControl(3, ssprintf("%lg", SS.showChordTol));
    SS.TW.edit.meaning = EDIT_CHORD_TOLERANCE;
    SS.TW.edit.i = 0;
}

void TextWindow::ScreenChangeMaxSegments(int link, uint32_t v) {
    SS.TW.ShowEditControl(3, ssprintf("%d", SS.maxSegments));
    SS.TW.edit.meaning = EDIT_MAX_SEGMENTS;
    SS.TW.edit.i = 0;
}

void TextWindow::ScreenChangeExportChordTolerance(int link, uint32_t v) {
    SS.TW.ShowEditControl(3, ssprintf("%lg", SS.exportChordTol));
    SS.TW.edit.meaning = EDIT_CHORD_TOLERANCE;
    SS.TW.edit.i = 1;
}

void TextWindow::ScreenChangeExportMaxSegments(int link, uint32_t v) {
    SS.TW.ShowEditControl(3, ssprintf("%d", SS.exportMaxSegments));
    SS.TW.edit.meaning = EDIT_MAX_SEGMENTS;
    SS.TW.edit.i = 1;
}

void TextWindow::ScreenChangeCameraTangent(int link, uint32_t v) {
    SS.TW.ShowEditControl(3, ssprintf("%.3f", 1000*SS.cameraTangent));
    SS.TW.edit.meaning = EDIT_CAMERA_TANGENT;
}

void TextWindow::ScreenChangeGridSpacing(int link, uint32_t v) {
    SS.TW.ShowEditControl(3, SS.MmToString(SS.gridSpacing));
    SS.TW.edit.meaning = EDIT_GRID_SPACING;
}

void TextWindow::ScreenChangeDigitsAfterDecimal(int link, uint32_t v) {
    SS.TW.ShowEditControl(3, ssprintf("%d", SS.UnitDigitsAfterDecimal()));
    SS.TW.edit.meaning = EDIT_DIGITS_AFTER_DECIMAL;
}

void TextWindow::ScreenChangeExportScale(int link, uint32_t v) {
    SS.TW.ShowEditControl(5, ssprintf("%.3f", (double)SS.exportScale));
    SS.TW.edit.meaning = EDIT_EXPORT_SCALE;
}

void TextWindow::ScreenChangeExportOffset(int link, uint32_t v) {
    SS.TW.ShowEditControl(3, SS.MmToString(SS.exportOffset));
    SS.TW.edit.meaning = EDIT_EXPORT_OFFSET;
}

void TextWindow::ScreenChangeFixExportColors(int link, uint32_t v) {
    SS.fixExportColors = !SS.fixExportColors;
}

void TextWindow::ScreenChangeBackFaces(int link, uint32_t v) {
    SS.drawBackFaces = !SS.drawBackFaces;
    InvalidateGraphics();
}

void TextWindow::ScreenChangeTurntableNav(int link, uint32_t v) {
    SS.turntableNav = !SS.turntableNav;
    if(SS.turntableNav) {
        // If turntable nav is being turned on, align view so Z is vertical
        SS.GW.AnimateOnto(Quaternion::From(Vector::From(-1, 0, 0), Vector::From(0, 0, 1)),
                          SS.GW.offset);
    }
}

void TextWindow::ScreenChangeImmediatelyEditDimension(int link, uint32_t v) {
    SS.immediatelyEditDimension = !SS.immediatelyEditDimension;
    InvalidateGraphics();
}

void TextWindow::ScreenChangeCheckClosedContour(int link, uint32_t v) {
    SS.checkClosedContour = !SS.checkClosedContour;
    InvalidateGraphics();
}

void TextWindow::ScreenChangeShadedTriangles(int link, uint32_t v) {
    SS.exportShadedTriangles = !SS.exportShadedTriangles;
    InvalidateGraphics();
}

void TextWindow::ScreenChangePwlCurves(int link, uint32_t v) {
    SS.exportPwlCurves = !SS.exportPwlCurves;
    InvalidateGraphics();
}

void TextWindow::ScreenChangeCanvasSizeAuto(int link, uint32_t v) {
    if(link == 't') {
        SS.exportCanvasSizeAuto = true;
    } else {
        SS.exportCanvasSizeAuto = false;
    }
    InvalidateGraphics();
}

void TextWindow::ScreenChangeCanvasSize(int link, uint32_t v) {
    double d;
    switch(v) {
        case  0: d = SS.exportMargin.left;      break;
        case  1: d = SS.exportMargin.right;     break;
        case  2: d = SS.exportMargin.bottom;    break;
        case  3: d = SS.exportMargin.top;       break;

        case 10: d = SS.exportCanvas.width;     break;
        case 11: d = SS.exportCanvas.height;    break;
        case 12: d = SS.exportCanvas.dx;        break;
        case 13: d = SS.exportCanvas.dy;        break;

        default: return;
    }
    int col = 13;
    if(v < 10) col = 11;
    SS.TW.ShowEditControl(col, SS.MmToString(d));
    SS.TW.edit.meaning = EDIT_CANVAS_SIZE;
    SS.TW.edit.i = v;
}

void TextWindow::ScreenChangeGCodeParameter(int link, uint32_t v) {
    std::string buf;
	char buffer [ 24 ];
    switch(link) {
        case 'd':
            SS.TW.edit.meaning = EDIT_G_CODE_DEPTH;
            buf += SS.MmToString(SS.gCode.depth);
            break;

        case 's':
            SS.TW.edit.meaning = EDIT_G_CODE_PASSES;
			_itoa( SS.gCode.passes, buffer, 10 );
			buf += buffer;//buf += std::to_string(SS.gCode.passes);
            break;

        case 'F':
            SS.TW.edit.meaning = EDIT_G_CODE_FEED;
            buf += SS.MmToString(SS.gCode.feed);
            break;

        case 'P':
            SS.TW.edit.meaning = EDIT_G_CODE_PLUNGE_FEED;
            buf += SS.MmToString(SS.gCode.plungeFeed);
            break;
    }
    SS.TW.ShowEditControl(15, buf);
}

void TextWindow::ScreenChangeAutosaveInterval(int link, uint32_t v) {
	char buffer [ 24 ];
	_itoa( SS.autosaveInterval, buffer, 10 );
	SS.TW.ShowEditControl(3, buffer);
    SS.TW.edit.meaning = EDIT_AUTOSAVE_INTERVAL;
}

void TextWindow::ShowConfiguration(void) {
    int i;
    Printf(true, "%Ft 标准颜色 (r, g, b)");

    for(i = 0; i < SS.MODEL_COLORS; i++) {
        Printf(false, "%Bp   #%d:  %Bz  %Bp  (%@, %@, %@) %f%D%Ll%Fl[修改]%E",
            (i & 1) ? 'd' : 'a',
            i, &SS.modelColor[i],
            (i & 1) ? 'd' : 'a',
            SS.modelColor[i].redF(),
            SS.modelColor[i].greenF(),
            SS.modelColor[i].blueF(),
            &ScreenChangeColor, i);
    }

    Printf(false, "");
    Printf(false, "%Ft 光向                             强度");
    for(i = 0; i < 2; i++) {
        Printf(false, "%Bp   #%d  (%2,%2,%2)%Fl%D%f%Ll[修改]%E "
                      "%2 %Fl%D%f%Ll[修改]%E",
            (i & 1) ? 'd' : 'a', i,
            CO(SS.lightDir[i]), i, &ScreenChangeLightDirection,
            SS.lightIntensity[i], i, &ScreenChangeLightIntensity);
    }

    Printf(false, "");
    Printf(false, "%Ft 显示细节度 (百分比)%E");
    Printf(false, "%Ba   %@ %% %Fl%Ll%f%D[修改]%E; %@ 毫米, %d 三角形",
        SS.showChordTol,
        &ScreenChangeChordTolerance, 0, SS.chordTolCalculated,
        SK.GetGroup(SS.GW.activeGroup)->displayMesh.l.n);
    Printf(false, "%Ft 最大线性分段%E");
    Printf(false, "%Ba   %d %Fl%Ll%f[修改]%E",
        SS.maxSegments,
        &ScreenChangeMaxSegments);

    Printf(false, "");
    Printf(false, "%Ft 导出文件细节度 (毫米)%E");
    Printf(false, "%Ba   %@ %Fl%Ll%f%D[修改]%E",
        SS.exportChordTol,
        &ScreenChangeExportChordTolerance, 0);
    Printf(false, "%Ft 导出文件最大线性分段%E");
    Printf(false, "%Ba   %d %Fl%Ll%f[修改]%E",
        SS.exportMaxSegments,
        &ScreenChangeExportMaxSegments);

    Printf(false, "");
    Printf(false, "%Ft 透视因子 (0=平行)%E");
    Printf(false, "%Ba   %# %Fl%Ll%f%D[修改]%E",
        SS.cameraTangent*1000,
        &ScreenChangeCameraTangent, 0);
    Printf(false, "%Ft 网格间距%E");
    Printf(false, "%Ba   %s %Fl%Ll%f%D[修改]%E",
        SS.MmToString(SS.gridSpacing).c_str(),
        &ScreenChangeGridSpacing, 0);
    Printf(false, "%Ft 显示小数位数%E");
    Printf(false, "%Ba   %d %Fl%Ll%f%D[修改]%E (e.g. '%s')",
        SS.UnitDigitsAfterDecimal(),
        &ScreenChangeDigitsAfterDecimal, 0,
        SS.MmToString(SS.StringToMm("1.23456789")).c_str());

    Printf(false, "");
    Printf(false, "%Ft 导出尺寸系数 (1:1=毫米, 1:25.4=英尺)");
    Printf(false, "%Ba   1:%# %Fl%Ll%f%D[修改]%E",
        (double)SS.exportScale,
        &ScreenChangeExportScale, 0);
    Printf(false, "%Ft 刀具半径补偿 (0=无效) ");
    Printf(false, "%Ba   %s %Fl%Ll%f%D[修改]%E",
        SS.MmToString(SS.exportOffset).c_str(),
        &ScreenChangeExportOffset, 0);

    Printf(false, "");
    Printf(false, "  %Fd%f%Ll%s  导出着色的2D三角形%E",
        &ScreenChangeShadedTriangles,
        SS.exportShadedTriangles ? CHECK_TRUE : CHECK_FALSE);
    if(fabs(SS.exportOffset) > LENGTH_EPS) {
        Printf(false, "  %Fd%s  以线段方式导出曲线%E "
                      "(刀具补偿为非零)", CHECK_TRUE);
    } else {
        Printf(false, "  %Fd%f%Ll%s  以线段方式导出曲线%E",
            &ScreenChangePwlCurves,
            SS.exportPwlCurves ? CHECK_TRUE : CHECK_FALSE);
    }
    Printf(false, "  %Fd%f%Ll%s  修复导出的白色线%E",
        &ScreenChangeFixExportColors,
        SS.fixExportColors ? CHECK_TRUE : CHECK_FALSE);

    Printf(false, "");
    Printf(false, "%Ft 导出画布大小:  "
                  "%f%Fd%Lf%s 固定%E  "
                  "%f%Fd%Lt%s 自动%E",
        &ScreenChangeCanvasSizeAuto,
        !SS.exportCanvasSizeAuto ? RADIO_TRUE : RADIO_FALSE,
        &ScreenChangeCanvasSizeAuto,
        SS.exportCanvasSizeAuto ? RADIO_TRUE : RADIO_FALSE);

    if(SS.exportCanvasSizeAuto) {
        Printf(false, "%Ft (按导出几何体周围的边距)");
        Printf(false, "%Ba%Ft   左:     %Fd%s %Fl%Ll%f%D[修改]%E",
            SS.MmToString(SS.exportMargin.left).c_str(), &ScreenChangeCanvasSize, 0);
        Printf(false, "%Bd%Ft   右:     %Fd%s %Fl%Ll%f%D[修改]%E",
            SS.MmToString(SS.exportMargin.right).c_str(), &ScreenChangeCanvasSize, 1);
        Printf(false, "%Ba%Ft   下:     %Fd%s %Fl%Ll%f%D[修改]%E",
            SS.MmToString(SS.exportMargin.bottom).c_str(), &ScreenChangeCanvasSize, 2);
        Printf(false, "%Bd%Ft   上:     %Fd%s %Fl%Ll%f%D[修改]%E",
            SS.MmToString(SS.exportMargin.top).c_str(), &ScreenChangeCanvasSize, 3);
    } else {
        Printf(false, "%Ft (按绝对尺寸和偏移量)");
        Printf(false, "%Ba%Ft   宽:     %Fd%s %Fl%Ll%f%D[修改]%E",
            SS.MmToString(SS.exportCanvas.width).c_str(), &ScreenChangeCanvasSize, 10);
        Printf(false, "%Bd%Ft   高:     %Fd%s %Fl%Ll%f%D[修改]%E",
            SS.MmToString(SS.exportCanvas.height).c_str(), &ScreenChangeCanvasSize, 11);
        Printf(false, "%Ba%Ft   x 偏移: %Fd%s %Fl%Ll%f%D[修改]%E",
            SS.MmToString(SS.exportCanvas.dx).c_str(), &ScreenChangeCanvasSize, 12);
        Printf(false, "%Bd%Ft   y 偏移: %Fd%s %Fl%Ll%f%D[修改]%E",
            SS.MmToString(SS.exportCanvas.dy).c_str(), &ScreenChangeCanvasSize, 13);
    }

    Printf(false, "");
    Printf(false, "%Ft G代码导出参数");
    Printf(false, "%Ba%Ft   深度:       %Fd%s %Fl%Ld%f[修改]%E",
        SS.MmToString(SS.gCode.depth).c_str(), &ScreenChangeGCodeParameter);
    Printf(false, "%Bd%Ft   遍数:       %Fd%d %Fl%Ls%f[修改]%E",
        SS.gCode.passes, &ScreenChangeGCodeParameter);
    Printf(false, "%Ba%Ft   快速进给:   %Fd%s %Fl%LF%f[修改]%E",
        SS.MmToString(SS.gCode.feed).c_str(), &ScreenChangeGCodeParameter);
    Printf(false, "%Bd%Ft   加工进给:   %Fd%s %Fl%LP%f[修改]%E",
        SS.MmToString(SS.gCode.plungeFeed).c_str(), &ScreenChangeGCodeParameter);

    Printf(false, "");
    Printf(false, "  %Fd%f%Ll%s  三角形背面以红色绘制%E",
        &ScreenChangeBackFaces,
        SS.drawBackFaces ? CHECK_TRUE : CHECK_FALSE);
    Printf(false, "  %Fd%f%Ll%s  检查草图的闭合轮廓%E",
        &ScreenChangeCheckClosedContour,
        SS.checkClosedContour ? CHECK_TRUE : CHECK_FALSE);
    Printf(false, "  %Fd%f%Ll%s  使用 SketchUp 鼠标导航模式%E",
        &ScreenChangeTurntableNav,
        SS.turntableNav ? CHECK_TRUE : CHECK_FALSE);
    Printf(false, "  %Fd%f%Ll%s  自动编辑新增约束%E",
        &ScreenChangeImmediatelyEditDimension,
        SS.immediatelyEditDimension ? CHECK_TRUE : CHECK_FALSE);

    Printf(false, "");
    Printf(false, "%Ft 自动保存间隔 (分钟)%E");
    Printf(false, "%Ba   %d %Fl%Ll%f[修改]%E",
        SS.autosaveInterval, &ScreenChangeAutosaveInterval);

    Printf(false, "");
    Printf(false, " %FtGL 供应商   %E%s", glGetString(GL_VENDOR));
    Printf(false, " %Ft   渲染器   %E%s", glGetString(GL_RENDERER));
    Printf(false, " %Ft   版本     %E%s", glGetString(GL_VERSION));
}

bool TextWindow::EditControlDoneForConfiguration(const char *s) {
    switch(edit.meaning) {
        case EDIT_LIGHT_INTENSITY:
            SS.lightIntensity[edit.i] = min(1.0, max(0.0, atof(s)));
            InvalidateGraphics();
            break;

        case EDIT_LIGHT_DIRECTION: {
            double x, y, z;
            if(sscanf(s, "%lf, %lf, %lf", &x, &y, &z)==3) {
                SS.lightDir[edit.i] = Vector::From(x, y, z);
            } else {
                Error("格式错误：将坐标指定为 x, y, z");
            }
            InvalidateGraphics();
            break;
        }
        case EDIT_COLOR: {
            Vector rgb;
            if(sscanf(s, "%lf, %lf, %lf", &rgb.x, &rgb.y, &rgb.z)==3) {
                rgb = rgb.ClampWithin(0, 1);
                SS.modelColor[edit.i] = RGBf(rgb.x, rgb.y, rgb.z);
            } else {
                Error("格式错误：将颜色指定为 r, g, b");
            }
            break;
        }
        case EDIT_CHORD_TOLERANCE: {
            if(edit.i == 0) {
                SS.showChordTol = max(0.0, atof(s));
                SS.GenerateAll(SolveSpaceUI::GENERATE_ALL);
            } else {
                SS.exportChordTol = max(0.0, atof(s));
            }
            break;
        }
        case EDIT_MAX_SEGMENTS: {
            if(edit.i == 0) {
                SS.maxSegments = min(1000, max(7, atoi(s)));
                SS.GenerateAll(SolveSpaceUI::GENERATE_ALL);
            } else {
                SS.exportMaxSegments = min(1000, max(7, atoi(s)));
            }
            break;
        }
        case EDIT_CAMERA_TANGENT: {
            SS.cameraTangent = (min(2.0, max(0.0, atof(s))))/1000.0;
            if(!SS.usePerspectiveProj) {
                Message("在您启用查看->使用透视投影之前，透视因子将不起作用。");
            }
            InvalidateGraphics();
            break;
        }
        case EDIT_GRID_SPACING: {
            SS.gridSpacing = (float)min(1e4, max(1e-3, SS.StringToMm(s)));
            InvalidateGraphics();
            break;
        }
        case EDIT_DIGITS_AFTER_DECIMAL: {
            int v = atoi(s);
            if(v < 0 || v > 8) {
                Error("使用 0 到 8 位之间的数字。");
            } else {
                SS.SetUnitDigitsAfterDecimal(v);
            }
            InvalidateGraphics();
            break;
        }
        case EDIT_EXPORT_SCALE: {
            Expr *e = Expr::From(s, true);
            if(e) {
                double ev = e->Eval();
                if(fabs(ev) < 0.001 || isnan(ev)) {
                    Error("导出比例不能为零！");
                } else {
                    SS.exportScale = (float)ev;
                }
            }
            break;
        }
        case EDIT_EXPORT_OFFSET: {
            Expr *e = Expr::From(s, true);
            if(e) {
                double ev = SS.ExprToMm(e);
                if(isnan(ev) || ev < 0) {
                    Error("刀具半径补偿不能为负！");
                } else {
                    SS.exportOffset = (float)ev;
                }
            }
            break;
        }
        case EDIT_CANVAS_SIZE: {
            Expr *e = Expr::From(s, true);
            if(!e) {
                break;
            }
            float d = (float)SS.ExprToMm(e);
            switch(edit.i) {
                case  0: SS.exportMargin.left   = d;    break;
                case  1: SS.exportMargin.right  = d;    break;
                case  2: SS.exportMargin.bottom = d;    break;
                case  3: SS.exportMargin.top    = d;    break;

                case 10: SS.exportCanvas.width  = d;    break;
                case 11: SS.exportCanvas.height = d;    break;
                case 12: SS.exportCanvas.dx     = d;    break;
                case 13: SS.exportCanvas.dy     = d;    break;
            }
            break;
        }
        case EDIT_G_CODE_DEPTH: {
            Expr *e = Expr::From(s, true);
            if(e) SS.gCode.depth = (float)SS.ExprToMm(e);
            break;
        }
        case EDIT_G_CODE_PASSES: {
            Expr *e = Expr::From(s, true);
            if(e) SS.gCode.passes = (int)(e->Eval());
            SS.gCode.passes = max(1, min(1000, SS.gCode.passes));
            break;
        }
        case EDIT_G_CODE_FEED: {
            Expr *e = Expr::From(s, true);
            if(e) SS.gCode.feed = (float)SS.ExprToMm(e);
            break;
        }
        case EDIT_G_CODE_PLUNGE_FEED: {
            Expr *e = Expr::From(s, true);
            if(e) SS.gCode.plungeFeed = (float)SS.ExprToMm(e);
            break;
        }
        case EDIT_AUTOSAVE_INTERVAL: {
            int interval;
            if(sscanf(s, "%d", &interval)==1) {
                if(interval >= 1) {
                    SS.autosaveInterval = interval;
                    SetAutosaveTimerFor(interval);
                } else {
                    Error("错误的值：自动保存间隔应为正数。");
                }
            } else {
                Error("格式错误：以整数分钟为单位指定间隔。");
            }
            break;
        }

        default: return false;
    }
    return true;
}

