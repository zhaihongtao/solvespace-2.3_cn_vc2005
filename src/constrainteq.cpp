//-----------------------------------------------------------------------------
// Given a constraint, generate one or more equations in our symbolic algebra
// system to represent that constraint; also various geometric helper
// functions for that.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

const hConstraint ConstraintBase::NO_CONSTRAINT = { 0 };

bool ConstraintBase::HasLabel(void) {
    switch(type) {
        case PT_LINE_DISTANCE:
        case PT_PLANE_DISTANCE:
        case PT_FACE_DISTANCE:
        case PT_PT_DISTANCE:
        case PROJ_PT_DISTANCE:
        case DIAMETER:
        case LENGTH_RATIO:
        case LENGTH_DIFFERENCE:
        case ANGLE:
        case COMMENT:
            return true;

        default:
            return false;
    }
}

Expr *ConstraintBase::VectorsParallel(int eq, ExprVector a, ExprVector b) {
    ExprVector r = a.Cross(b);
    // Hairy ball theorem screws me here. There's no clean solution that I
    // know, so let's pivot on the initial numerical guess. Our caller
    // has ensured that if one of our input vectors is already known (e.g.
    // it's from a previous group), then that one's in a; so that one's
    // not going to move, and we should pivot on that one.
    double mx = fabs((a.x)->Eval());
    double my = fabs((a.y)->Eval());
    double mz = fabs((a.z)->Eval());
    // The basis vector in which the vectors have the LEAST energy is the
    // one that we should look at most (e.g. if both vectors lie in the xy
    // plane, then the z component of the cross product is most important).
    // So find the strongest component of a and b, and that's the component
    // of the cross product to ignore.
    Expr *e0, *e1;
    if(mx > my && mx > mz) {
        e0 = r.y; e1 = r.z;
    } else if(my > mz) {
        e0 = r.z; e1 = r.x;
    } else {
        e0 = r.x; e1 = r.y;
    }

    if(eq == 0) return e0;
    if(eq == 1) return e1;
    oops();
}

Expr *ConstraintBase::PointLineDistance(hEntity wrkpl, hEntity hpt, hEntity hln)
{
    EntityBase *ln = SK.GetEntity(hln);
    EntityBase *a = SK.GetEntity(ln->point[0]);
    EntityBase *b = SK.GetEntity(ln->point[1]);

    EntityBase *p = SK.GetEntity(hpt);

    if(wrkpl.v == EntityBase::FREE_IN_3D.v) {
        ExprVector ep = p->PointGetExprs();

        ExprVector ea = a->PointGetExprs();
        ExprVector eb = b->PointGetExprs();
        ExprVector eab = ea.Minus(eb);
        Expr *m = eab.Magnitude();

        return ((eab.Cross(ea.Minus(ep))).Magnitude())->Div(m);
    } else {
        Expr *ua, *va, *ub, *vb;
        a->PointGetExprsInWorkplane(wrkpl, &ua, &va);
        b->PointGetExprsInWorkplane(wrkpl, &ub, &vb);

        Expr *du = ua->Minus(ub);
        Expr *dv = va->Minus(vb);

        Expr *u, *v;
        p->PointGetExprsInWorkplane(wrkpl, &u, &v);

        Expr *m = ((du->Square())->Plus(dv->Square()))->Sqrt();

        Expr *proj = (dv->Times(ua->Minus(u)))->Minus(
                     (du->Times(va->Minus(v))));

        return proj->Div(m);
    }
}

Expr *ConstraintBase::PointPlaneDistance(ExprVector p, hEntity hpl) {
    ExprVector n;
    Expr *d;
    SK.GetEntity(hpl)->WorkplaneGetPlaneExprs(&n, &d);
    return (p.Dot(n))->Minus(d);
}

Expr *ConstraintBase::Distance(hEntity wrkpl, hEntity hpa, hEntity hpb) {
    EntityBase *pa = SK.GetEntity(hpa);
    EntityBase *pb = SK.GetEntity(hpb);
    if(!(pa->IsPoint() && pb->IsPoint())) oops();

    if(wrkpl.v == EntityBase::FREE_IN_3D.v) {
        // This is true distance
        ExprVector ea, eb, eab;
        ea = pa->PointGetExprs();
        eb = pb->PointGetExprs();
        eab = ea.Minus(eb);

        return eab.Magnitude();
    } else {
        // This is projected distance, in the given workplane.
        Expr *au, *av, *bu, *bv;

        pa->PointGetExprsInWorkplane(wrkpl, &au, &av);
        pb->PointGetExprsInWorkplane(wrkpl, &bu, &bv);

        Expr *du = au->Minus(bu);
        Expr *dv = av->Minus(bv);

        return ((du->Square())->Plus(dv->Square()))->Sqrt();
    }
}

//-----------------------------------------------------------------------------
// Return the cosine of the angle between two vectors. If a workplane is
// specified, then it's the cosine of their projections into that workplane.
//-----------------------------------------------------------------------------
Expr *ConstraintBase::DirectionCosine(hEntity wrkpl,
                                      ExprVector ae, ExprVector be)
{
    if(wrkpl.v == EntityBase::FREE_IN_3D.v) {
        Expr *mags = (ae.Magnitude())->Times(be.Magnitude());
        return (ae.Dot(be))->Div(mags);
    } else {
        EntityBase *w = SK.GetEntity(wrkpl);
        ExprVector u = w->Normal()->NormalExprsU();
        ExprVector v = w->Normal()->NormalExprsV();
        Expr *ua = u.Dot(ae);
        Expr *va = v.Dot(ae);
        Expr *ub = u.Dot(be);
        Expr *vb = v.Dot(be);
        Expr *maga = (ua->Square()->Plus(va->Square()))->Sqrt();
        Expr *magb = (ub->Square()->Plus(vb->Square()))->Sqrt();
        Expr *dot = (ua->Times(ub))->Plus(va->Times(vb));
        return dot->Div(maga->Times(magb));
    }
}

ExprVector ConstraintBase::PointInThreeSpace(hEntity workplane,
                                             Expr *u, Expr *v)
{
    EntityBase *w = SK.GetEntity(workplane);

    ExprVector ub = w->Normal()->NormalExprsU();
    ExprVector vb = w->Normal()->NormalExprsV();
    ExprVector ob = w->WorkplaneGetOffsetExprs();

    return (ub.ScaledBy(u)).Plus(vb.ScaledBy(v)).Plus(ob);
}

void ConstraintBase::ModifyToSatisfy(void) {
    if(type == ANGLE) {
        Vector a = SK.GetEntity(entityA)->VectorGetNum();
        Vector b = SK.GetEntity(entityB)->VectorGetNum();
        if(other) a = a.ScaledBy(-1);
        if(workplane.v != EntityBase::FREE_IN_3D.v) {
            a = a.ProjectVectorInto(workplane);
            b = b.ProjectVectorInto(workplane);
        }
        double c = (a.Dot(b))/(a.Magnitude() * b.Magnitude());
        valA = acos(c)*180/PI;
    } else {
        // We'll fix these ones up by looking at their symbolic equation;
        // that means no extra work.
        IdList<Equation,hEquation> l = {};
        // Generate the equations even if this is a reference dimension
        GenerateReal(&l);
        if(l.n != 1) oops();

        // These equations are written in the form f(...) - d = 0, where
        // d is the value of the valA.
        valA += (l.elem[0].e)->Eval();

        l.Clear();
    }
}

void ConstraintBase::AddEq(IdList<Equation,hEquation> *l, Expr *expr, int index)
{
    Equation eq;
    eq.e = expr;
    eq.h = h.equation(index);
    l->Add(&eq);
}

void ConstraintBase::Generate(IdList<Equation,hEquation> *l) {
    if(!reference) {
        GenerateReal(l);
    }
}
void ConstraintBase::GenerateReal(IdList<Equation,hEquation> *l) {
    Expr *exA = Expr::From(valA);

    switch(type) {
        case PT_PT_DISTANCE:
            AddEq(l, Distance(workplane, ptA, ptB)->Minus(exA), 0);
            break;

        case PROJ_PT_DISTANCE: {
            ExprVector pA = SK.GetEntity(ptA)->PointGetExprs(),
                       pB = SK.GetEntity(ptB)->PointGetExprs(),
                       dp = pB.Minus(pA);

            ExprVector pp = SK.GetEntity(entityA)->VectorGetExprs();
            pp = pp.WithMagnitude(Expr::From(1.0));

            AddEq(l, (dp.Dot(pp))->Minus(exA), 0);
            break;
        }

        case PT_LINE_DISTANCE:
            AddEq(l,
                PointLineDistance(workplane, ptA, entityA)->Minus(exA), 0);
            break;

        case PT_PLANE_DISTANCE: {
            ExprVector pt = SK.GetEntity(ptA)->PointGetExprs();
            AddEq(l, (PointPlaneDistance(pt, entityA))->Minus(exA), 0);
            break;
        }

        case PT_FACE_DISTANCE: {
            ExprVector pt = SK.GetEntity(ptA)->PointGetExprs();
            EntityBase *f = SK.GetEntity(entityA);
            ExprVector p0 = f->FaceGetPointExprs();
            ExprVector n = f->FaceGetNormalExprs();
            AddEq(l, (pt.Minus(p0)).Dot(n)->Minus(exA), 0);
            break;
        }

        case EQUAL_LENGTH_LINES: {
            EntityBase *a = SK.GetEntity(entityA);
            EntityBase *b = SK.GetEntity(entityB);
            AddEq(l, Distance(workplane, a->point[0], a->point[1])->Minus(
                     Distance(workplane, b->point[0], b->point[1])), 0);
            break;
        }

        // These work on distance squared, since the pt-line distances are
        // signed, and we want the absolute value.
        case EQ_LEN_PT_LINE_D: {
            EntityBase *forLen = SK.GetEntity(entityA);
            Expr *d1 = Distance(workplane, forLen->point[0], forLen->point[1]);
            Expr *d2 = PointLineDistance(workplane, ptA, entityB);
            AddEq(l, (d1->Square())->Minus(d2->Square()), 0);
            break;
        }
        case EQ_PT_LN_DISTANCES: {
            Expr *d1 = PointLineDistance(workplane, ptA, entityA);
            Expr *d2 = PointLineDistance(workplane, ptB, entityB);
            AddEq(l, (d1->Square())->Minus(d2->Square()), 0);
            break;
        }

        case LENGTH_RATIO: {
            EntityBase *a = SK.GetEntity(entityA);
            EntityBase *b = SK.GetEntity(entityB);
            Expr *la = Distance(workplane, a->point[0], a->point[1]);
            Expr *lb = Distance(workplane, b->point[0], b->point[1]);
            AddEq(l, (la->Div(lb))->Minus(exA), 0);
            break;
        }

        case LENGTH_DIFFERENCE: {
            EntityBase *a = SK.GetEntity(entityA);
            EntityBase *b = SK.GetEntity(entityB);
            Expr *la = Distance(workplane, a->point[0], a->point[1]);
            Expr *lb = Distance(workplane, b->point[0], b->point[1]);
            AddEq(l, (la->Minus(lb))->Minus(exA), 0);
            break;
        }

        case DIAMETER: {
            EntityBase *circle = SK.GetEntity(entityA);
            Expr *r = circle->CircleGetRadiusExpr();
            AddEq(l, (r->Times(Expr::From(2)))->Minus(exA), 0);
            break;
        }

        case EQUAL_RADIUS: {
            EntityBase *c1 = SK.GetEntity(entityA);
            EntityBase *c2 = SK.GetEntity(entityB);
            AddEq(l, (c1->CircleGetRadiusExpr())->Minus(
                      c2->CircleGetRadiusExpr()), 0);
            break;
        }

        case EQUAL_LINE_ARC_LEN: {
            EntityBase *line = SK.GetEntity(entityA),
                       *arc  = SK.GetEntity(entityB);

            // Get the line length
            ExprVector l0 = SK.GetEntity(line->point[0])->PointGetExprs(),
                       l1 = SK.GetEntity(line->point[1])->PointGetExprs();
            Expr *ll = (l1.Minus(l0)).Magnitude();

            // And get the arc radius, and the cosine of its angle
            EntityBase *ao = SK.GetEntity(arc->point[0]),
                       *as = SK.GetEntity(arc->point[1]),
                       *af = SK.GetEntity(arc->point[2]);

            ExprVector aos = (as->PointGetExprs()).Minus(ao->PointGetExprs()),
                       aof = (af->PointGetExprs()).Minus(ao->PointGetExprs());
            Expr *r = aof.Magnitude();

            ExprVector n = arc->Normal()->NormalExprsN();
            ExprVector u = aos.WithMagnitude(Expr::From(1.0));
            ExprVector v = n.Cross(u);
            // so in our new csys, we start at (1, 0, 0)
            Expr *costheta = aof.Dot(u)->Div(r);
            Expr *sintheta = aof.Dot(v)->Div(r);

            double thetas, thetaf, dtheta;
            arc->ArcGetAngles(&thetas, &thetaf, &dtheta);
            Expr *theta;
            if(dtheta < 3*PI/4) {
                theta = costheta->ACos();
            } else if(dtheta < 5*PI/4) {
                // As the angle crosses pi, cos theta is not invertible;
                // so use the sine to stop blowing up
                theta = Expr::From(PI)->Minus(sintheta->ASin());
            } else {
                theta = (Expr::From(2*PI))->Minus(costheta->ACos());
            }

            // And write the equation; r*theta = L
            AddEq(l, (r->Times(theta))->Minus(ll), 0);
            break;
        }

        case POINTS_COINCIDENT: {
            EntityBase *a = SK.GetEntity(ptA);
            EntityBase *b = SK.GetEntity(ptB);
            if(workplane.v == EntityBase::FREE_IN_3D.v) {
                ExprVector pa = a->PointGetExprs();
                ExprVector pb = b->PointGetExprs();
                AddEq(l, pa.x->Minus(pb.x), 0);
                AddEq(l, pa.y->Minus(pb.y), 1);
                AddEq(l, pa.z->Minus(pb.z), 2);
            } else {
                Expr *au, *av;
                Expr *bu, *bv;
                a->PointGetExprsInWorkplane(workplane, &au, &av);
                b->PointGetExprsInWorkplane(workplane, &bu, &bv);
                AddEq(l, au->Minus(bu), 0);
                AddEq(l, av->Minus(bv), 1);
            }
            break;
        }

        case PT_IN_PLANE:
            // This one works the same, whether projected or not.
            AddEq(l, PointPlaneDistance(
                        SK.GetEntity(ptA)->PointGetExprs(), entityA), 0);
            break;

        case PT_ON_FACE: {
            // a plane, n dot (p - p0) = 0
            ExprVector p = SK.GetEntity(ptA)->PointGetExprs();
            EntityBase *f = SK.GetEntity(entityA);
            ExprVector p0 = f->FaceGetPointExprs();
            ExprVector n = f->FaceGetNormalExprs();
            AddEq(l, (p.Minus(p0)).Dot(n), 0);
            break;
        }

        case PT_ON_LINE:
            if(workplane.v == EntityBase::FREE_IN_3D.v) {
                EntityBase *ln = SK.GetEntity(entityA);
                EntityBase *a = SK.GetEntity(ln->point[0]);
                EntityBase *b = SK.GetEntity(ln->point[1]);
                EntityBase *p = SK.GetEntity(ptA);

                ExprVector ep = p->PointGetExprs();
                ExprVector ea = a->PointGetExprs();
                ExprVector eb = b->PointGetExprs();
                ExprVector eab = ea.Minus(eb);

                // Construct a vector from the point to either endpoint of
                // the line segment, and choose the longer of these.
                ExprVector eap = ea.Minus(ep);
                ExprVector ebp = eb.Minus(ep);
                ExprVector elp =
                    (ebp.Magnitude()->Eval() > eap.Magnitude()->Eval()) ?
                        ebp : eap;

                if(p->group.v == group.v) {
                    AddEq(l, VectorsParallel(0, eab, elp), 0);
                    AddEq(l, VectorsParallel(1, eab, elp), 1);
                } else {
                    AddEq(l, VectorsParallel(0, elp, eab), 0);
                    AddEq(l, VectorsParallel(1, elp, eab), 1);
                }
            } else {
                AddEq(l, PointLineDistance(workplane, ptA, entityA), 0);
            }
            break;

        case PT_ON_CIRCLE: {
            // This actually constrains the point to lie on the cylinder.
            EntityBase *circle = SK.GetEntity(entityA);
            ExprVector center = SK.GetEntity(circle->point[0])->PointGetExprs();
            ExprVector pt     = SK.GetEntity(ptA)->PointGetExprs();
            EntityBase *normal = SK.GetEntity(circle->normal);
            ExprVector u = normal->NormalExprsU(),
                       v = normal->NormalExprsV();

            Expr *du = (center.Minus(pt)).Dot(u),
                 *dv = (center.Minus(pt)).Dot(v);

            Expr *r = circle->CircleGetRadiusExpr();

            //AddEq(l, ((du->Square())->Plus(dv->Square()))->Minus(r->Square()), 0);
            AddEq(l, du->Square()->Plus(dv->Square())->Sqrt()->Minus(r), 0);
            break;
        }

        case AT_MIDPOINT:
            if(workplane.v == EntityBase::FREE_IN_3D.v) {
                EntityBase *ln = SK.GetEntity(entityA);
                ExprVector a = SK.GetEntity(ln->point[0])->PointGetExprs();
                ExprVector b = SK.GetEntity(ln->point[1])->PointGetExprs();
                ExprVector m = (a.Plus(b)).ScaledBy(Expr::From(0.5));

                if(ptA.v) {
                    ExprVector p = SK.GetEntity(ptA)->PointGetExprs();
                    AddEq(l, (m.x)->Minus(p.x), 0);
                    AddEq(l, (m.y)->Minus(p.y), 1);
                    AddEq(l, (m.z)->Minus(p.z), 2);
                } else {
                    AddEq(l, PointPlaneDistance(m, entityB), 0);
                }
            } else {
                EntityBase *ln = SK.GetEntity(entityA);
                EntityBase *a = SK.GetEntity(ln->point[0]);
                EntityBase *b = SK.GetEntity(ln->point[1]);

                Expr *au, *av, *bu, *bv;
                a->PointGetExprsInWorkplane(workplane, &au, &av);
                b->PointGetExprsInWorkplane(workplane, &bu, &bv);
                Expr *mu = Expr::From(0.5)->Times(au->Plus(bu));
                Expr *mv = Expr::From(0.5)->Times(av->Plus(bv));

                if(ptA.v) {
                    EntityBase *p = SK.GetEntity(ptA);
                    Expr *pu, *pv;
                    p->PointGetExprsInWorkplane(workplane, &pu, &pv);
                    AddEq(l, pu->Minus(mu), 0);
                    AddEq(l, pv->Minus(mv), 1);
                } else {
                    ExprVector m = PointInThreeSpace(workplane, mu, mv);
                    AddEq(l, PointPlaneDistance(m, entityB), 0);
                }
            }
            break;

        case SYMMETRIC:
            if(workplane.v == EntityBase::FREE_IN_3D.v) {
                EntityBase *plane = SK.GetEntity(entityA);
                EntityBase *ea = SK.GetEntity(ptA);
                EntityBase *eb = SK.GetEntity(ptB);
                ExprVector a = ea->PointGetExprs();
                ExprVector b = eb->PointGetExprs();

                // The midpoint of the line connecting the symmetric points
                // lies on the plane of the symmetry.
                ExprVector m = (a.Plus(b)).ScaledBy(Expr::From(0.5));
                AddEq(l, PointPlaneDistance(m, plane->h), 0);

                // And projected into the plane of symmetry, the points are
                // coincident.
                Expr *au, *av, *bu, *bv;
                ea->PointGetExprsInWorkplane(plane->h, &au, &av);
                eb->PointGetExprsInWorkplane(plane->h, &bu, &bv);
                AddEq(l, au->Minus(bu), 1);
                AddEq(l, av->Minus(bv), 2);
            } else {
                EntityBase *plane = SK.GetEntity(entityA);
                EntityBase *a = SK.GetEntity(ptA);
                EntityBase *b = SK.GetEntity(ptB);

                Expr *au, *av, *bu, *bv;
                a->PointGetExprsInWorkplane(workplane, &au, &av);
                b->PointGetExprsInWorkplane(workplane, &bu, &bv);
                Expr *mu = Expr::From(0.5)->Times(au->Plus(bu));
                Expr *mv = Expr::From(0.5)->Times(av->Plus(bv));

                ExprVector m = PointInThreeSpace(workplane, mu, mv);
                AddEq(l, PointPlaneDistance(m, plane->h), 0);

                // Construct a vector within the workplane that is normal
                // to the symmetry pane's normal (i.e., that lies in the
                // plane of symmetry). The line connecting the points is
                // perpendicular to that constructed vector.
                EntityBase *w = SK.GetEntity(workplane);
                ExprVector u = w->Normal()->NormalExprsU();
                ExprVector v = w->Normal()->NormalExprsV();

                ExprVector pa = a->PointGetExprs();
                ExprVector pb = b->PointGetExprs();
                ExprVector n;
                Expr *d;
                plane->WorkplaneGetPlaneExprs(&n, &d);
                AddEq(l, (n.Cross(u.Cross(v))).Dot(pa.Minus(pb)), 1);
            }
            break;

        case SYMMETRIC_HORIZ:
        case SYMMETRIC_VERT: {
            EntityBase *a = SK.GetEntity(ptA);
            EntityBase *b = SK.GetEntity(ptB);

            Expr *au, *av, *bu, *bv;
            a->PointGetExprsInWorkplane(workplane, &au, &av);
            b->PointGetExprsInWorkplane(workplane, &bu, &bv);

            if(type == SYMMETRIC_HORIZ) {
                AddEq(l, av->Minus(bv), 0);
                AddEq(l, au->Plus(bu), 1);
            } else {
                AddEq(l, au->Minus(bu), 0);
                AddEq(l, av->Plus(bv), 1);
            }
            break;
        }

        case SYMMETRIC_LINE: {
            EntityBase *pa = SK.GetEntity(ptA);
            EntityBase *pb = SK.GetEntity(ptB);

            Expr *pau, *pav, *pbu, *pbv;
            pa->PointGetExprsInWorkplane(workplane, &pau, &pav);
            pb->PointGetExprsInWorkplane(workplane, &pbu, &pbv);

            EntityBase *ln = SK.GetEntity(entityA);
            EntityBase *la = SK.GetEntity(ln->point[0]);
            EntityBase *lb = SK.GetEntity(ln->point[1]);
            Expr *lau, *lav, *lbu, *lbv;
            la->PointGetExprsInWorkplane(workplane, &lau, &lav);
            lb->PointGetExprsInWorkplane(workplane, &lbu, &lbv);

            Expr *dpu = pbu->Minus(pau), *dpv = pbv->Minus(pav);
            Expr *dlu = lbu->Minus(lau), *dlv = lbv->Minus(lav);

            // The line through the points is perpendicular to the line
            // of symmetry.
            AddEq(l, (dlu->Times(dpu))->Plus(dlv->Times(dpv)), 0);

            // And the signed distances of the points to the line are
            // equal in magnitude and opposite in sign, so sum to zero
            Expr *dista = (dlv->Times(lau->Minus(pau)))->Minus(
                          (dlu->Times(lav->Minus(pav))));
            Expr *distb = (dlv->Times(lau->Minus(pbu)))->Minus(
                          (dlu->Times(lav->Minus(pbv))));
            AddEq(l, dista->Plus(distb), 1);

            break;
        }

        case HORIZONTAL:
        case VERTICAL: {
            hEntity ha, hb;
            if(entityA.v) {
                EntityBase *e = SK.GetEntity(entityA);
                ha = e->point[0];
                hb = e->point[1];
            } else {
                ha = ptA;
                hb = ptB;
            }
            EntityBase *a = SK.GetEntity(ha);
            EntityBase *b = SK.GetEntity(hb);

            Expr *au, *av, *bu, *bv;
            a->PointGetExprsInWorkplane(workplane, &au, &av);
            b->PointGetExprsInWorkplane(workplane, &bu, &bv);

            AddEq(l, (type == HORIZONTAL) ? av->Minus(bv) : au->Minus(bu), 0);
            break;
        }

        case SAME_ORIENTATION: {
            EntityBase *a = SK.GetEntity(entityA);
            EntityBase *b = SK.GetEntity(entityB);
            if(b->group.v != group.v) {
                swap(a, b);
            }

            ExprVector au = a->NormalExprsU(),
                       an = a->NormalExprsN();
            ExprVector bu = b->NormalExprsU(),
                       bv = b->NormalExprsV(),
                       bn = b->NormalExprsN();

            AddEq(l, VectorsParallel(0, an, bn), 0);
            AddEq(l, VectorsParallel(1, an, bn), 1);
            Expr *d1 = au.Dot(bv);
            Expr *d2 = au.Dot(bu);
            // Allow either orientation for the coordinate system, depending
            // on how it was drawn.
            if(fabs(d1->Eval()) < fabs(d2->Eval())) {
                AddEq(l, d1, 2);
            } else {
                AddEq(l, d2, 2);
            }
            break;
        }

        case PERPENDICULAR:
        case ANGLE: {
            EntityBase *a = SK.GetEntity(entityA);
            EntityBase *b = SK.GetEntity(entityB);
            ExprVector ae = a->VectorGetExprs();
            ExprVector be = b->VectorGetExprs();
            if(other) ae = ae.ScaledBy(Expr::From(-1));
            Expr *c = DirectionCosine(workplane, ae, be);

            if(type == ANGLE) {
                // The direction cosine is equal to the cosine of the
                // specified angle
                Expr *rads = exA->Times(Expr::From(PI/180)),
                     *rc   = rads->Cos();
                double arc = fabs(rc->Eval());
                // avoid false detection of inconsistent systems by gaining
                // up as the difference in dot products gets small at small
                // angles; doubles still have plenty of precision, only
                // problem is that rank test
                Expr *mult = Expr::From(arc > 0.99 ? 0.01/(1.00001 - arc) : 1);
                AddEq(l, (c->Minus(rc))->Times(mult), 0);
            } else {
                // The dot product (and therefore the direction cosine)
                // is equal to zero, perpendicular.
                AddEq(l, c, 0);
            }
            break;
        }

        case EQUAL_ANGLE: {
            EntityBase *a = SK.GetEntity(entityA);
            EntityBase *b = SK.GetEntity(entityB);
            EntityBase *c = SK.GetEntity(entityC);
            EntityBase *d = SK.GetEntity(entityD);
            ExprVector ae = a->VectorGetExprs();
            ExprVector be = b->VectorGetExprs();
            ExprVector ce = c->VectorGetExprs();
            ExprVector de = d->VectorGetExprs();

            if(other) ae = ae.ScaledBy(Expr::From(-1));

            Expr *cab = DirectionCosine(workplane, ae, be);
            Expr *ccd = DirectionCosine(workplane, ce, de);

            AddEq(l, cab->Minus(ccd), 0);
            break;
        }

        case ARC_LINE_TANGENT: {
            EntityBase *arc  = SK.GetEntity(entityA);
            EntityBase *line = SK.GetEntity(entityB);

            ExprVector ac = SK.GetEntity(arc->point[0])->PointGetExprs();
            ExprVector ap =
                SK.GetEntity(arc->point[other ? 2 : 1])->PointGetExprs();

            ExprVector ld = line->VectorGetExprs();

            // The line is perpendicular to the radius
            AddEq(l, ld.Dot(ac.Minus(ap)), 0);
            break;
        }

        case CUBIC_LINE_TANGENT: {
            EntityBase *cubic = SK.GetEntity(entityA);
            EntityBase *line  = SK.GetEntity(entityB);

            ExprVector a;
            if(other) {
                a = cubic->CubicGetFinishTangentExprs();
            } else {
                a = cubic->CubicGetStartTangentExprs();
            }

            ExprVector b = line->VectorGetExprs();

            if(workplane.v == EntityBase::FREE_IN_3D.v) {
                AddEq(l, VectorsParallel(0, a, b), 0);
                AddEq(l, VectorsParallel(1, a, b), 1);
            } else {
                EntityBase *w = SK.GetEntity(workplane);
                ExprVector wn = w->Normal()->NormalExprsN();
                AddEq(l, (a.Cross(b)).Dot(wn), 0);
            }
            break;
        }

        case CURVE_CURVE_TANGENT: {
            bool parallel = true;
            int i;
            ExprVector dir[2];
            for(i = 0; i < 2; i++) {
                EntityBase *e = SK.GetEntity((i == 0) ? entityA : entityB);
                bool oth = (i == 0) ? other : other2;

                if(e->type == Entity::ARC_OF_CIRCLE) {
                    ExprVector center, endpoint;
                    center = SK.GetEntity(e->point[0])->PointGetExprs();
                    endpoint =
                        SK.GetEntity(e->point[oth ? 2 : 1])->PointGetExprs();
                    dir[i] = endpoint.Minus(center);
                    // We're using the vector from the center of the arc to
                    // an endpoint; so that's normal to the tangent, not
                    // parallel.
                    parallel = !parallel;
                } else if(e->type == Entity::CUBIC) {
                    if(oth) {
                        dir[i] = e->CubicGetFinishTangentExprs();
                    } else {
                        dir[i] = e->CubicGetStartTangentExprs();
                    }
                } else {
                    oops();
                }
            }
            if(parallel) {
                EntityBase *w = SK.GetEntity(workplane);
                ExprVector wn = w->Normal()->NormalExprsN();
                AddEq(l, ((dir[0]).Cross(dir[1])).Dot(wn), 0);
            } else {
                AddEq(l, (dir[0]).Dot(dir[1]), 0);
            }
            break;
        }

        case PARALLEL: {
            EntityBase *ea = SK.GetEntity(entityA), *eb = SK.GetEntity(entityB);
            if(eb->group.v != group.v) {
                swap(ea, eb);
            }
            ExprVector a = ea->VectorGetExprs();
            ExprVector b = eb->VectorGetExprs();

            if(workplane.v == EntityBase::FREE_IN_3D.v) {
                AddEq(l, VectorsParallel(0, a, b), 0);
                AddEq(l, VectorsParallel(1, a, b), 1);
            } else {
                EntityBase *w = SK.GetEntity(workplane);
                ExprVector wn = w->Normal()->NormalExprsN();
                AddEq(l, (a.Cross(b)).Dot(wn), 0);
            }
            break;
        }

        case WHERE_DRAGGED: {
            EntityBase *ep = SK.GetEntity(ptA);
            if(workplane.v == EntityBase::FREE_IN_3D.v) {
                ExprVector ev = ep->PointGetExprs();
                Vector v = ep->PointGetNum();

                AddEq(l, ev.x->Minus(Expr::From(v.x)), 0);
                AddEq(l, ev.y->Minus(Expr::From(v.y)), 1);
                AddEq(l, ev.z->Minus(Expr::From(v.z)), 2);
            } else {
                Expr *u, *v;
                ep->PointGetExprsInWorkplane(workplane, &u, &v);
                AddEq(l, u->Minus(Expr::From(u->Eval())), 0);
                AddEq(l, v->Minus(Expr::From(v->Eval())), 1);
            }
            break;
        }

        case COMMENT:
            break;

        default: oops();
    }
}

