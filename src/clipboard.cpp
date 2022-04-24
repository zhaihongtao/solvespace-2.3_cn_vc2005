//-----------------------------------------------------------------------------
// The clipboard that gets manipulated when the user selects Edit -> Cut,
// Copy, Paste, etc.; may contain entities only, not constraints.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

void SolveSpaceUI::Clipboard::Clear(void) {
    c.Clear();
    r.Clear();
}


bool SolveSpaceUI::Clipboard::ContainsEntity(hEntity he) {
    if(he.v == Entity::NO_ENTITY.v)
        return true;

    ClipboardRequest *cr;
    for(cr = r.First(); cr; cr = r.NextAfter(cr)) {
        if(cr->oldEnt.v == he.v)
            return true;

        for(int i = 0; i < MAX_POINTS_IN_ENTITY; i++) {
            if(cr->oldPointEnt[i].v == he.v)
                return true;
        }
    }
    return false;
}

hEntity SolveSpaceUI::Clipboard::NewEntityFor(hEntity he) {
    if(he.v == Entity::NO_ENTITY.v)
        return Entity::NO_ENTITY;

    ClipboardRequest *cr;
    for(cr = r.First(); cr; cr = r.NextAfter(cr)) {
        if(cr->oldEnt.v == he.v)
            return cr->newReq.entity(0);

        for(int i = 0; i < MAX_POINTS_IN_ENTITY; i++) {
            if(cr->oldPointEnt[i].v == he.v)
                return cr->newReq.entity(1+i);
        }
    }
    oops();
}

void GraphicsWindow::DeleteSelection(void) {
    SK.request.ClearTags();
    SK.constraint.ClearTags();
    List<Selection> *ls = &(selection);
    for(Selection *s = ls->First(); s; s = ls->NextAfter(s)) {
        hRequest r = { 0 };
        if(s->entity.v && s->entity.isFromRequest()) {
            r = s->entity.request();
        }
        if(r.v && !r.IsFromReferences()) {
            SK.request.Tag(r, 1);
        }
        if(s->constraint.v) {
            SK.constraint.Tag(s->constraint, 1);
        }
    }

    SK.constraint.RemoveTagged();
    // Note that this regenerates and clears the selection, to avoid
    // lingering references to the just-deleted items.
    DeleteTaggedRequests();
}

void GraphicsWindow::CopySelection(void) {
    SS.clipboard.Clear();

    Entity *wrkpl  = SK.GetEntity(ActiveWorkplane());
    Entity *wrkpln = SK.GetEntity(wrkpl->normal);
    Vector u = wrkpln->NormalU(),
           v = wrkpln->NormalV(),
           n = wrkpln->NormalN(),
           p = SK.GetEntity(wrkpl->point[0])->PointGetNum();

    List<Selection> *ls = &(selection);
    for(Selection *s = ls->First(); s; s = ls->NextAfter(s)) {
        if(!s->entity.v) continue;
        // Work only on entities that have requests that will generate them.
        Entity *e = SK.GetEntity(s->entity);
        bool hasDistance;
        int req, pts;
        if(!EntReqTable::GetEntityInfo(e->type, e->extraPoints,
                &req, &pts, NULL, &hasDistance))
        {
            if(!e->h.isFromRequest()) continue;
            Request *r = SK.GetRequest(e->h.request());
            if(r->type != Request::DATUM_POINT) continue;
            EntReqTable::GetEntityInfo(0, e->extraPoints,
                &req, &pts, NULL, &hasDistance);
        }
        if(req == Request::WORKPLANE) continue;

        ClipboardRequest cr = {};
        cr.type         = req;
        cr.extraPoints  = e->extraPoints;
        cr.style        = e->style;
        cr.str          = e->str;
        cr.font         = e->font;
        cr.construction = e->construction;
        {for(int i = 0; i < pts; i++) {
            Vector pt;
            if(req == Request::DATUM_POINT) {
                pt = e->PointGetNum();
            } else {
                pt = SK.GetEntity(e->point[i])->PointGetNum();
            }
            pt = pt.Minus(p);
            pt = pt.DotInToCsys(u, v, n);
            cr.point[i] = pt;
        }}
        if(hasDistance) {
            cr.distance = SK.GetEntity(e->distance)->DistanceGetNum();
        }

        cr.oldEnt = e->h;
        for(int i = 0; i < pts; i++) {
            cr.oldPointEnt[i] = e->point[i];
        }

        SS.clipboard.r.Add(&cr);
    }

    for(Selection *s = ls->First(); s; s = ls->NextAfter(s)) {
        if(!s->constraint.v) continue;

        Constraint *c = SK.GetConstraint(s->constraint);
        if(c->type == Constraint::COMMENT) {
            SS.clipboard.c.Add(c);
        }
    }

    Constraint *c;
    for(c = SK.constraint.First(); c; c = SK.constraint.NextAfter(c)) {
        if(!SS.clipboard.ContainsEntity(c->ptA) ||
           !SS.clipboard.ContainsEntity(c->ptB) ||
           !SS.clipboard.ContainsEntity(c->entityA) ||
           !SS.clipboard.ContainsEntity(c->entityB) ||
           !SS.clipboard.ContainsEntity(c->entityC) ||
           !SS.clipboard.ContainsEntity(c->entityD) ||
           c->type == Constraint::COMMENT) {
            continue;
        }
        SS.clipboard.c.Add(c);
    }
}

hEntity GraphicsWindow::mapPoint ( hEntity he, double scale )
{
	if(he.v == 0) return he;
	if(scale < 0) {
		hRequest hr = he.request();
		Request *r = SK.GetRequest(hr);
		if(r->type == Request::ARC_OF_CIRCLE) {
			if(he.v == hr.entity(2).v) {
				return hr.entity(3);
			} else if(he.v == hr.entity(3).v) {
				return hr.entity(2);
			}
		}
	}
	return he;
}

void GraphicsWindow::PasteClipboard(Vector trans, double theta, double scale) {
    Entity *wrkpl  = SK.GetEntity(ActiveWorkplane());
    Entity *wrkpln = SK.GetEntity(wrkpl->normal);
    Vector u = wrkpln->NormalU(),
           v = wrkpln->NormalV(),
           n = wrkpln->NormalN(),
           p = SK.GetEntity(wrkpl->point[0])->PointGetNum();

    // For arcs, reflection involves swapping the endpoints, or otherwise
    // the arc gets inverted.
//     auto mapPoint = [scale](hEntity he) {
//         if(he.v == 0) return he;
//         if(scale < 0) {
//             hRequest hr = he.request();
//             Request *r = SK.GetRequest(hr);
//             if(r->type == Request::ARC_OF_CIRCLE) {
//                 if(he.v == hr.entity(2).v) {
//                     return hr.entity(3);
//                 } else if(he.v == hr.entity(3).v) {
//                     return hr.entity(2);
//                 }
//             }
//         }
//         return he;
//     };

    ClipboardRequest *cr;
    for(cr = SS.clipboard.r.First(); cr; cr = SS.clipboard.r.NextAfter(cr)) {
        hRequest hr = AddRequest(cr->type, false);
        Request *r = SK.GetRequest(hr);
        r->extraPoints  = cr->extraPoints;
        r->style        = cr->style;
        r->str          = cr->str;
        r->font         = cr->font;
        r->construction = cr->construction;
        // Need to regen to get the right number of points, if extraPoints
        // changed.
        SS.GenerateAll(SolveSpaceUI::GENERATE_REGEN);
        SS.MarkGroupDirty(r->group);
        bool hasDistance;
        int i, pts;
        EntReqTable::GetRequestInfo(r->type, r->extraPoints,
            NULL, &pts, NULL, &hasDistance);
        for(i = 0; i < pts; i++) {
            Vector pt = cr->point[i];
            // We need the reflection to occur within the workplane; it may
            // otherwise correspond to just a rotation as projected.
            if(scale < 0) {
                pt.x *= -1;
            }
            // Likewise the scale, which could otherwise take us out of the
            // workplane.
            pt = pt.ScaledBy(fabs(scale));
            pt = pt.ScaleOutOfCsys(u, v, Vector::From(0, 0, 0));
            pt = pt.Plus(p);
            pt = pt.RotatedAbout(n, theta);
            pt = pt.Plus(trans);
            int j = (r->type == Request::DATUM_POINT) ? i : i + 1;
            SK.GetEntity(hr.entity(j))->PointForceTo(pt);
        }
        if(hasDistance) {
            SK.GetEntity(hr.entity(64))->DistanceForceTo(
                                            cr->distance*fabs(scale));
        }

        cr->newReq = hr;
        MakeSelected(hr.entity(0));
        for(i = 0; i < pts; i++) {
            int j = (r->type == Request::DATUM_POINT) ? i : i + 1;
            MakeSelected(hr.entity(j));
        }
    }

    Constraint *cc;
    for(cc = SS.clipboard.c.First(); cc; cc = SS.clipboard.c.NextAfter(cc)) {
        Constraint c; ZERO(&c);
        c.group = SS.GW.activeGroup;
        c.workplane = SS.GW.ActiveWorkplane();
        c.type = cc->type;
        c.valA = cc->valA;
        c.ptA = SS.clipboard.NewEntityFor(mapPoint(cc->ptA, scale));
        c.ptB = SS.clipboard.NewEntityFor(mapPoint(cc->ptB, scale));
        c.entityA = SS.clipboard.NewEntityFor(cc->entityA);
        c.entityB = SS.clipboard.NewEntityFor(cc->entityB);
        c.entityC = SS.clipboard.NewEntityFor(cc->entityC);
        c.entityD = SS.clipboard.NewEntityFor(cc->entityD);
        c.other = cc->other;
        c.other2 = cc->other2;
        c.reference = cc->reference;
        c.disp = cc->disp;
        c.comment = cc->comment;
        switch(c.type) {
            case Constraint::COMMENT:
                c.disp.offset = c.disp.offset.Plus(trans);
                break;

            case Constraint::PT_PT_DISTANCE:
            case Constraint::PT_LINE_DISTANCE:
            case Constraint::PROJ_PT_DISTANCE:
            case Constraint::DIAMETER:
                c.valA *= fabs(scale);
                break;

            default:
                break;
        }

        hConstraint hc = Constraint::AddConstraint(&c, false);//rememberForUndo=
        if(c.type == Constraint::COMMENT) {
            MakeSelected(hc);
        }
    }

    SS.ScheduleGenerateAll();
}

void GraphicsWindow::MenuClipboard(int id) {
    if(id != MNU_DELETE && !SS.GW.LockedInWorkplane()) {
        Error("ֻ���ڹ���ƽ���м��С�ճ���͸��ƹ�����\n\n"
              "��ѡ���ͼ -> ƽ���ͼ��");
        return;
    }

    switch(id) {
        case MNU_PASTE: {
            SS.UndoRemember();
            Vector trans = SS.GW.projRight.ScaledBy(80/SS.GW.scale).Plus(
                           SS.GW.projUp   .ScaledBy(40/SS.GW.scale));
            SS.GW.ClearSelection();
            SS.GW.PasteClipboard(trans, 0, 1);
            break;
        }

        case MNU_PASTE_TRANSFORM: {
            if(SS.clipboard.r.n == 0) {
                Error("������Ϊ�գ�û�п�ճ�������ݡ�");
                break;
            }

            Entity *wrkpl  = SK.GetEntity(SS.GW.ActiveWorkplane());
            Vector p = SK.GetEntity(wrkpl->point[0])->PointGetNum();
            SS.TW.shown.paste.times  = 1;
            SS.TW.shown.paste.trans  = Vector::From(0, 0, 0);
            SS.TW.shown.paste.theta  = 0;
            SS.TW.shown.paste.origin = p;
            SS.TW.shown.paste.scale  = 1;
            SS.TW.GoToScreen(TextWindow::SCREEN_PASTE_TRANSFORMED);
            SS.GW.ForceTextWindowShown();
            SS.ScheduleShowTW();
            break;
        }

        case MNU_COPY:
            SS.GW.CopySelection();
            SS.GW.ClearSelection();
            break;

        case MNU_CUT:
            SS.UndoRemember();
            SS.GW.CopySelection();
            SS.GW.DeleteSelection();
            break;

        case MNU_DELETE:
            SS.UndoRemember();
            SS.GW.DeleteSelection();
            break;

        default: oops();
    }
}

bool TextWindow::EditControlDoneForPaste(const char *s) {
    Expr *e;
    switch(edit.meaning) {
        case EDIT_PASTE_TIMES_REPEATED: {
            e = Expr::From(s, true);
            if(!e) break;
            int v = (int)e->Eval();
            if(v > 0) {
                shown.paste.times = v;
            } else {
                Error("Ҫճ���ĸ���������������Ϊһ����");
            }
            break;
        }
        case EDIT_PASTE_ANGLE:
            e = Expr::From(s, true);
            if(!e) break;
            shown.paste.theta = WRAP_SYMMETRIC((e->Eval())*PI/180, 2*PI);
            break;

        case EDIT_PASTE_SCALE: {
            e = Expr::From(s, true);
            double v = e->Eval();
            if(fabs(v) > 1e-6) {
                shown.paste.scale = v;
            } else {
                Error("��������Ϊ�㡣");
            }
            break;
        }

        default:
            return false;
    }
    return true;
}

void TextWindow::ScreenChangePasteTransformed(int link, uint32_t v) {
    switch(link) {
        case 't':
            SS.TW.ShowEditControl(13, ssprintf("%d", SS.TW.shown.paste.times));
            SS.TW.edit.meaning = EDIT_PASTE_TIMES_REPEATED;
            break;

        case 'r':
            SS.TW.ShowEditControl(13, ssprintf("%.3f", SS.TW.shown.paste.theta*180/PI));
            SS.TW.edit.meaning = EDIT_PASTE_ANGLE;
            break;

        case 's':
            SS.TW.ShowEditControl(13, ssprintf("%.3f", SS.TW.shown.paste.scale));
            SS.TW.edit.meaning = EDIT_PASTE_SCALE;
            break;
    }
}

void TextWindow::ScreenPasteTransformed(int link, uint32_t v) {
	SS.GW.GroupSelection();
	switch(link) {
		case 'o':
			if(SS.GW.gs.points == 1 && SS.GW.gs.n == 1) {
				Entity *e = SK.GetEntity(SS.GW.gs.point[0]);
				SS.TW.shown.paste.origin = e->PointGetNum();
			} else {
				Error("ѡ��һ�����Զ�����ת��ԭ�㡣");
			}
			SS.GW.ClearSelection();
			break;

		case 't':
			if(SS.GW.gs.points == 2 && SS.GW.gs.n == 2) {
				Entity *pa = SK.GetEntity(SS.GW.gs.point[0]),
					*pb = SK.GetEntity(SS.GW.gs.point[1]);
				SS.TW.shown.paste.trans =
					(pb->PointGetNum()).Minus(pa->PointGetNum());
			} else {
				Error("ѡ���������Զ���ƽ��ʸ����");
			}
			SS.GW.ClearSelection();
			break;

		case 'g': {
			if(fabs(SS.TW.shown.paste.theta) < LENGTH_EPS &&
				SS.TW.shown.paste.trans.Magnitude() < LENGTH_EPS &&
				SS.TW.shown.paste.times != 1)
			{
				Message("�任�ǵ�λ����������и���������ȷ���ص������������ϡ�");
			}
			if(SS.TW.shown.paste.times*SS.clipboard.r.n > 100) {
				Error("Ҫճ������Ŀ̫�࣬�뽫����Ϊ��С����Ŀ��");
				break;
			}
			if(!SS.GW.LockedInWorkplane()) {
				Error("û�м���Ĺ����档");
				break;
			}
			Entity *wrkpl  = SK.GetEntity(SS.GW.ActiveWorkplane());
			Entity *wrkpln = SK.GetEntity(wrkpl->normal);
			Vector wn = wrkpln->NormalN();
			SS.UndoRemember();
			SS.GW.ClearSelection();
			for(int i = 0; i < SS.TW.shown.paste.times; i++) {
				Vector trans  = SS.TW.shown.paste.trans.ScaledBy(i+1),
					origin = SS.TW.shown.paste.origin;
				double theta = SS.TW.shown.paste.theta*(i+1);
				// desired transformation is Q*(p - o) + o + t =
				// Q*p - Q*o + o + t = Q*p + (t + o - Q*o)
				Vector t = trans.Plus(
					origin).Minus(
					origin.RotatedAbout(wn, theta));

				SS.GW.PasteClipboard(t, theta, SS.TW.shown.paste.scale);
			}
			SS.TW.GoToScreen(SCREEN_LIST_OF_GROUPS);
			SS.ScheduleShowTW();
			break;
				  }
	}
}


