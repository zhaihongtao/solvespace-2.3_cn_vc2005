//-----------------------------------------------------------------------------
// The screens when an entity is selected, that show some description of it--
// endpoints of the lines, diameter of the circle, etc.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

void TextWindow::ScreenUnselectAll(int link, uint32_t v) {
    GraphicsWindow::MenuEdit(GraphicsWindow::MNU_UNSELECT_ALL);
}

void TextWindow::ScreenEditTtfText(int link, uint32_t v) {
    hRequest hr = { v };
    Request *r = SK.GetRequest(hr);

    SS.TW.ShowEditControl(10, r->str);
    SS.TW.edit.meaning = EDIT_TTF_TEXT;
    SS.TW.edit.request = hr;
}

#define gs (SS.GW.gs)
void TextWindow::ScreenSetTtfFont(int link, uint32_t v) {
    int i = (int)v;
    if(i < 0) return;
    if(i >= SS.fonts.l.n) return;

    SS.GW.GroupSelection();
    if(gs.entities != 1 || gs.n != 1) return;

    Entity *e = SK.entity.FindByIdNoOops(gs.entity[0]);
    if(!e || e->type != Entity::TTF_TEXT || !e->h.isFromRequest()) return;

    Request *r = SK.request.FindByIdNoOops(e->h.request());
    if(!r) return;

    SS.UndoRemember();
    r->font = SS.fonts.l.elem[i].FontFileBaseName();
    SS.MarkGroupDirty(r->group);
    SS.ScheduleGenerateAll();
    SS.ScheduleShowTW();
}

void TextWindow::ScreenConstraintShowAsRadius(int link, uint32_t v) {
    hConstraint hc = { v };
    Constraint *c = SK.GetConstraint(hc);

    SS.UndoRemember();
    c->other = !c->other;

    SS.ScheduleShowTW();
}

bool is_no_entity ( hEntity he ){
	return he.v == Entity::NO_ENTITY.v || !he.isFromRequest();
}

void TextWindow::DescribeSelection(void) {
    Printf(false, "");

    if(gs.n == 1 && (gs.points == 1 || gs.entities == 1)) {
        Entity *e = SK.GetEntity(gs.points == 1 ? gs.point[0] : gs.entity[0]);
        Vector p;

#define COSTR(p) \
    SS.MmToString((p).x).c_str(), \
    SS.MmToString((p).y).c_str(), \
    SS.MmToString((p).z).c_str()
#define PT_AS_STR "(%Fi%s%E, %Fi%s%E, %Fi%s%E)"
#define PT_AS_NUM "(%Fi%3%E, %Fi%3%E, %Fi%3%E)"

        switch(e->type) {
            case Entity::POINT_IN_3D:
            case Entity::POINT_IN_2D:
            case Entity::POINT_N_TRANS:
            case Entity::POINT_N_ROT_TRANS:
            case Entity::POINT_N_COPY:
            case Entity::POINT_N_ROT_AA:
                p = e->PointGetNum();
                Printf(false, "%Ft点%E at " PT_AS_STR, COSTR(p));
                break;

            case Entity::NORMAL_IN_3D:
            case Entity::NORMAL_IN_2D:
            case Entity::NORMAL_N_COPY:
            case Entity::NORMAL_N_ROT:
            case Entity::NORMAL_N_ROT_AA: {
                Quaternion q = e->NormalGetNum();
                p = q.RotationN();
                Printf(false, "%Ft法线 / 坐标系%E");
                Printf(true,  "  基准 n = " PT_AS_NUM, CO(p));
                p = q.RotationU();
                Printf(false, "       u = " PT_AS_NUM, CO(p));
                p = q.RotationV();
                Printf(false, "       v = " PT_AS_NUM, CO(p));
                break;
            }
            case Entity::WORKPLANE: {
                p = SK.GetEntity(e->point[0])->PointGetNum();
                Printf(false, "%Ft工作平面%E");
                Printf(true, " 原点 = " PT_AS_STR, COSTR(p));
                Quaternion q = e->Normal()->NormalGetNum();
                p = q.RotationN();
                Printf(true, " 法线 = " PT_AS_NUM, CO(p));
                break;
            }
            case Entity::LINE_SEGMENT: {
                Vector p0 = SK.GetEntity(e->point[0])->PointGetNum();
                p = p0;
                Printf(false, "%Ft线段%E");
                Printf(true,  " 起点 " PT_AS_STR, COSTR(p));
                Vector p1 = SK.GetEntity(e->point[1])->PointGetNum();
                p = p1;
                Printf(false, "      " PT_AS_STR, COSTR(p));
                Printf(true,  " 长度 = %Fi%s%E",
                    SS.MmToString((p1.Minus(p0).Magnitude())).c_str());
                break;
            }
            case Entity::CUBIC_PERIODIC:
            case Entity::CUBIC:
                int pts;
                if(e->type == Entity::CUBIC_PERIODIC) {
                    Printf(false, "%Ft固定 C2 立方样条%E");
                    pts = (3 + e->extraPoints);
                } else if(e->extraPoints > 0) {
                    Printf(false, "%Ft插值 C2 立方样条%E");
                    pts = (4 + e->extraPoints);
                } else {
                    Printf(false, "%Ft立方样条%E");
                    pts = 4;
                }
                for(int i = 0; i < pts; i++) {
                    p = SK.GetEntity(e->point[i])->PointGetNum();
                    Printf((i==0), "   p%d = " PT_AS_STR, i, COSTR(p));
                }
                break;

            case Entity::ARC_OF_CIRCLE: {
                Printf(false, "%Ft圆弧%E");
                p = SK.GetEntity(e->point[0])->PointGetNum();
                Printf(true,  "  圆心 = " PT_AS_STR, COSTR(p));
                p = SK.GetEntity(e->point[1])->PointGetNum();
                Printf(true,  "  端点 = " PT_AS_STR, COSTR(p));
                p = SK.GetEntity(e->point[2])->PointGetNum();
                Printf(false, "              " PT_AS_STR, COSTR(p));
                double r = e->CircleGetRadiusNum();
                Printf(true,  "  直径 =  %Fi%s", SS.MmToString(r*2).c_str());
                Printf(false, "  半径 =  %Fi%s", SS.MmToString(r).c_str());
                double thetas, thetaf, dtheta;
                e->ArcGetAngles(&thetas, &thetaf, &dtheta);
                Printf(false, "  弧长 =  %Fi%s", SS.MmToString(dtheta*r).c_str());
                break;
            }
            case Entity::CIRCLE: {
                Printf(false, "%Ft圆形%E");
                p = SK.GetEntity(e->point[0])->PointGetNum();
                Printf(true,  "  圆心 = " PT_AS_STR, COSTR(p));
                double r = e->CircleGetRadiusNum();
                Printf(true,  "  直径 =  %Fi%s", SS.MmToString(r*2).c_str());
                Printf(false, "  半径 =  %Fi%s", SS.MmToString(r).c_str());
                break;
            }
            case Entity::FACE_NORMAL_PT:
            case Entity::FACE_XPROD:
            case Entity::FACE_N_ROT_TRANS:
            case Entity::FACE_N_ROT_AA:
            case Entity::FACE_N_TRANS:
                Printf(false, "%Ft平面%E");
                p = e->FaceGetNormalNum();
                Printf(true,  "  法线 = " PT_AS_NUM, CO(p));
                p = e->FaceGetPointNum();
                Printf(false, "  起点 = " PT_AS_STR, COSTR(p));
                break;

            case Entity::TTF_TEXT: {
                Printf(false, "%Ft文字%E");
                Printf(true, "  字体 = '%Fi%s%E'", e->font.c_str());
                if(e->h.isFromRequest()) {
                    Printf(false, "  文字 = '%Fi%s%E' %Fl%Ll%f%D[修改]%E",
                        e->str.c_str(), &ScreenEditTtfText, e->h.request().v);
                    Printf(true, "%Ft选择字体%E");
                    SS.fonts.LoadAll();
                    int i;
                    for(i = 0; i < SS.fonts.l.n; i++) {
                        TtfFont *tf = &(SS.fonts.l.elem[i]);
                        if(e->font == tf->FontFileBaseName()) {
                            Printf(false, "%Bp    %s",
                                (i & 1) ? 'd' : 'a',
                                tf->name.c_str());
                        } else {
                            Printf(false, "%Bp    %f%D%Fl%Ll%s%E%Bp",
                                (i & 1) ? 'd' : 'a',
                                &ScreenSetTtfFont, i,
                                tf->name.c_str(),
                                (i & 1) ? 'd' : 'a');
                        }
                    }
                } else {
                    Printf(false, "  文字 = '%Fi%s%E'", e->str.c_str());
                }
                break;
            }

            default:
                Printf(true, "%Ft?? ENTITY%E");
                break;
        }

        Group *g = SK.GetGroup(e->group);
        Printf(false, "");
        Printf(false, "%Ft组名%E      %s", g->DescriptionString().c_str());
        if(e->workplane.v == Entity::FREE_IN_3D.v) {
            Printf(false, "%Ft未在工作平面锁定%E");
        } else {
            Entity *w = SK.GetEntity(e->workplane);
            Printf(false, "%Ft工作平面%E  %s", w->DescriptionString().c_str());
        }
        if(e->style.v) {
            Style *s = Style::Get(e->style);
            Printf(false, "%Ft样式%E      %s", s->DescriptionString().c_str());
        } else {
            Printf(false, "%Ft样式%E      无");
        }
        if(e->construction) {
            Printf(false, "%Ft构造线");
        }

        std::vector<hConstraint> lhc;// = {};
        for(int i = 0; i < SK.constraint.n; i++) {
            Constraint *c = &(SK.constraint.elem[i]);
            if(!(c->ptA.v == e->h.v ||
                 c->ptB.v == e->h.v ||
                 c->entityA.v == e->h.v ||
                 c->entityB.v == e->h.v ||
                 c->entityC.v == e->h.v ||
                 c->entityD.v == e->h.v)) continue;
            lhc.push_back(c->h);
        }

        if(!lhc.empty()) {
            Printf(true, "%Ft约束:%E");

            int a = 0;
			int cnt = lhc.size ();
			for ( int i = 0; i < cnt; i ++ ) {
            //for(hConstraint hc : lhc) {
				hConstraint hc = lhc [ i ];
                Constraint *c = SK.GetConstraint(hc);
                std::string s = c->DescriptionString();
                Printf(false, "%Bp   %Fl%Ll%D%f%h%s%E %s",
                    (a & 1) ? 'd' : 'a',
                    c->h.v, (&TextWindow::ScreenSelectConstraint),
                    (&TextWindow::ScreenHoverConstraint), s.c_str(),
                    c->reference ? "(ref)" : "");
                a++;
            }
        }
    } else if(gs.n == 2 && gs.points == 2) {
        Printf(false, "%Ft两点");
        Vector p0 = SK.GetEntity(gs.point[0])->PointGetNum();
        Printf(true,  "   在 " PT_AS_STR, COSTR(p0));
        Vector p1 = SK.GetEntity(gs.point[1])->PointGetNum();
        Printf(false, "      " PT_AS_STR, COSTR(p1));
        double d = (p1.Minus(p0)).Magnitude();
        Printf(true, "  d = %Fi%s", SS.MmToString(d).c_str());
    } else if(gs.n == 2 && gs.points == 1 && gs.circlesOrArcs == 1) {
        Entity *ec = SK.GetEntity(gs.entity[0]);
        if(ec->type == Entity::CIRCLE) {
            Printf(false, "%Ft点和圆");
        } else if(ec->type == Entity::ARC_OF_CIRCLE) {
            Printf(false, "%Ft点和圆弧");
        } else oops();
        Vector p = SK.GetEntity(gs.point[0])->PointGetNum();
        Printf(true,  "  点坐标 " PT_AS_STR, COSTR(p));
        Vector c = SK.GetEntity(ec->point[0])->PointGetNum();
        Printf(true,  "  中心 = " PT_AS_STR, COSTR(c));
        double r = ec->CircleGetRadiusNum();
        Printf(false, "  直径 =  %Fi%s", SS.MmToString(r*2).c_str());
        Printf(false, "  半径 =  %Fi%s", SS.MmToString(r).c_str());
        double d = (p.Minus(c)).Magnitude() - r;
        Printf(true,  "  距离 = %Fi%s", SS.MmToString(d).c_str());
    } else if(gs.n == 2 && gs.faces == 1 && gs.points == 1) {
        Printf(false, "%Ft点和平面");
        Vector pt = SK.GetEntity(gs.point[0])->PointGetNum();
        Printf(true,  "        点 = " PT_AS_STR, COSTR(pt));
        Vector n = SK.GetEntity(gs.face[0])->FaceGetNormalNum();
        Printf(true,  "  平面法线 = " PT_AS_NUM, CO(n));
        Vector pl = SK.GetEntity(gs.face[0])->FaceGetPointNum();
        Printf(false, "  平面起点 = " PT_AS_STR, COSTR(pl));
        double dd = n.Dot(pl) - n.Dot(pt);
        Printf(true,  "      距离 = %Fi%s", SS.MmToString(dd).c_str());
    } else if(gs.n == 3 && gs.points == 2 && gs.vectors == 1) {
        Printf(false, "%Ft两点和向量");
        Vector p0 = SK.GetEntity(gs.point[0])->PointGetNum();
        Printf(true,  "   点A = " PT_AS_STR, COSTR(p0));
        Vector p1 = SK.GetEntity(gs.point[1])->PointGetNum();
        Printf(false, "   点B = " PT_AS_STR, COSTR(p1));
        Vector v  = SK.GetEntity(gs.vector[0])->VectorGetNum();
        v = v.WithMagnitude(1);
        Printf(true,  "  向量 = " PT_AS_NUM, CO(v));
        double d = (p1.Minus(p0)).Dot(v);
        Printf(true,  "  投影 = %Fi%s", SS.MmToString(d).c_str());
    } else if(gs.n == 2 && gs.lineSegments == 1 && gs.points == 1) {
        Entity *ln = SK.GetEntity(gs.entity[0]);
        Vector lp0 = SK.GetEntity(ln->point[0])->PointGetNum(),
               lp1 = SK.GetEntity(ln->point[1])->PointGetNum();
        Printf(false, "%Ft线段和点%E");
        Printf(true,  "  线通过 " PT_AS_STR, COSTR(lp0));
        Printf(false, "           " PT_AS_STR, COSTR(lp1));
        Vector pp = SK.GetEntity(gs.point[0])->PointGetNum();
        Printf(true,  "      点 " PT_AS_STR, COSTR(pp));
        Printf(true,  "  点线距离 = %Fi%s%E",
            SS.MmToString(pp.DistanceToLine(lp0, lp1.Minus(lp0))).c_str());
    } else if(gs.n == 2 && gs.vectors == 2) {
        Printf(false, "%Ft两个向量");

        Vector v0 = SK.GetEntity(gs.entity[0])->VectorGetNum(),
               v1 = SK.GetEntity(gs.entity[1])->VectorGetNum();
        v0 = v0.WithMagnitude(1);
        v1 = v1.WithMagnitude(1);

        Printf(true,  "   向量A = " PT_AS_NUM, CO(v0));
        Printf(false, "   向量B = " PT_AS_NUM, CO(v1));

        double theta = acos(v0.Dot(v1));
        Printf(true,  "    角度 = %Fi%2%E 度", theta*180/PI);
        while(theta < PI/2) theta += PI;
        while(theta > PI/2) theta -= PI;
        Printf(false, "  或角度 = %Fi%2%E (mod 180)", theta*180/PI);
    } else if(gs.n == 2 && gs.faces == 2) {
        Printf(false, "%Ft两个平面");

        Vector n0 = SK.GetEntity(gs.face[0])->FaceGetNormalNum();
        Printf(true,  " 平面A 法线 = " PT_AS_NUM, CO(n0));
        Vector p0 = SK.GetEntity(gs.face[0])->FaceGetPointNum();
        Printf(false, " 平面A 起点 = " PT_AS_STR, COSTR(p0));

        Vector n1 = SK.GetEntity(gs.face[1])->FaceGetNormalNum();
        Printf(true,  " 平面B 法线 = " PT_AS_NUM, CO(n1));
        Vector p1 = SK.GetEntity(gs.face[1])->FaceGetPointNum();
        Printf(false, " 平面B 起点 = " PT_AS_STR, COSTR(p1));

        double theta = acos(n0.Dot(n1));
        Printf(true,  "       角度 = %Fi%2%E 度", theta*180/PI);
        while(theta < PI/2) theta += PI;
        while(theta > PI/2) theta -= PI;
        Printf(false, "     或角度 = %Fi%2%E (mod 180)", theta*180/PI);

        if(fabs(theta) < 0.01) {
            double d = (p1.Minus(p0)).Dot(n0);
            Printf(true,  "       距离 = %Fi%s", SS.MmToString(d).c_str());
        }
    } else if(gs.n == 0 && gs.stylables > 0) {
        Printf(false, "%Ft选择:%E 标注文字");
    } else if(gs.n == 0 && gs.constraints == 1) {
        Constraint *c = SK.GetConstraint(gs.constraint[0]);

        if(c->type == Constraint::DIAMETER) {
            Printf(false, "%Ft圆标注");

            Printf(true, "  %Fd%f%D%Ll%s  显示半径",
                   &ScreenConstraintShowAsRadius, gs.constraint[0].v,
                   c->other ? CHECK_TRUE : CHECK_FALSE);
        } else {
            Printf(false, "%Ft选择:%E %s",
            c->DescriptionString().c_str());
        }

        std::vector<hEntity> lhe;// = {};
        lhe.push_back(c->ptA);
        lhe.push_back(c->ptB);
        lhe.push_back(c->entityA);
        lhe.push_back(c->entityB);
        lhe.push_back(c->entityC);
        lhe.push_back(c->entityD);



        std::vector<hEntity>::iterator it = std::remove_if(lhe.begin(), lhe.end(), is_no_entity );
            //[](hEntity he) {
            //    return he.v == Entity::NO_ENTITY.v || !he.isFromRequest();
            //});
        lhe.erase(it, lhe.end());

        if(!lhe.empty()) {
            Printf(true, "%Ft被约束对象:%E");

            int a = 0;
			int cnt = lhe.size ();
			for ( int i = 0; i < cnt; i ++ ) {
				hEntity he = lhe [ i ];
            //for(hEntity he : lhe) {
                Request *r = SK.GetRequest(he.request());
                std::string s = r->DescriptionString();
                Printf(false, "%Bp   %Fl%Ll%D%f%h%s%E",
                    (a & 1) ? 'd' : 'a',
                    r->h.v, (&TextWindow::ScreenSelectRequest),
                    &(TextWindow::ScreenHoverRequest), s.c_str());
                a++;
            }
        }
    } else {
        int n = SS.GW.selection.n;
        Printf(false, "%Ft已选择:%E %d 项%s", n, n == 1 ? "" : "s");
    }

    if(shown.screen == SCREEN_STYLE_INFO &&
       shown.style.v >= Style::FIRST_CUSTOM && gs.stylables > 0)
    {
        // If we are showing a screen for a particular style, then offer the
        // option to assign our selected entities to that style.
        Style *s = Style::Get(shown.style);
        Printf(true, "%Fl%D%f%Ll(分配样式 %s)%E",
            shown.style.v,
            &ScreenAssignSelectionToStyle,
            s->DescriptionString().c_str());
    }
    // If any of the selected entities have an assigned style, then offer
    // the option to remove that style.
    bool styleAssigned = false;
    for(int i = 0; i < gs.entities; i++) {
        Entity *e = SK.GetEntity(gs.entity[i]);
        if(e->style.v != 0) {
            styleAssigned = true;
        }
    }
    for(int i = 0; i < gs.constraints; i++) {
        Constraint *c = SK.GetConstraint(gs.constraint[i]);
        if(c->type == Constraint::COMMENT && c->disp.style.v != 0) {
            styleAssigned = true;
        }
    }
    if(styleAssigned) {
        Printf(true, "%Fl%D%f%Ll(删除分配的样式)%E",
            0,
            &ScreenAssignSelectionToStyle);
    }

    Printf(true, "%Fl%f%Ll(取消全选)%E", &TextWindow::ScreenUnselectAll);
}

void TextWindow::GoToScreen(int screen) {
    shown.screen = screen;
}

