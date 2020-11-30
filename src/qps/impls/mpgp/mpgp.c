
#include <../src/qps/impls/mpgp/mpgpimpl.h>

const char *const QPSMPGPExpansionTypes[] = {"std","projcg","gf","g","gfgr","ggr","QPSMPGPExpansionType","QPS_MPGP_EXPANSION_",0};
const char *const QPSMPGPExpansionLengthTypes[] = {"fixed","opt","optapprox","bb","QPSMPGPExpansionLengthType","QPS_MPGP_EXPANSION_LENGTH_",0};

/*
  WORK VECTORS:

  gP = qps->work[0];
  gf = qps->work[1];
  gc = qps->work[2];

  g  = qps->work[3];
  p  = qps->work[4];
  Ap = qps->work[5];
  gr = qps->work[6];
*/

#undef __FUNCT__
#define __FUNCT__ "QPSMonitorDefault_MPGP"
PetscErrorCode QPSMonitorDefault_MPGP(QPS qps,PetscInt n,PetscViewer viewer)
{
   QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

   PetscFunctionBegin;
   if (n == 0 && ((PetscObject)qps)->prefix) {
     TRY( PetscViewerASCIIPrintf(viewer,"  Projected gradient norms for %s solve.\n",((PetscObject)qps)->prefix) );
   }

   TRY( PetscViewerASCIIPrintf(viewer,"%3D MPGP [%c] ||gp||=%.10e",n,mpgp->currentStepType,(double)qps->rnorm) );
   TRY( PetscViewerASCIIPrintf(viewer,",\t||gf||=%.10e",(double)mpgp->gfnorm) );
   TRY( PetscViewerASCIIPrintf(viewer,",\t||gc||=%.10e",(double)mpgp->gcnorm) );
   TRY( PetscViewerASCIIPrintf(viewer,",\talpha=%.10e",(double)mpgp->alpha) );
   TRY( PetscViewerASCIIPrintf(viewer,"\n") );
   PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetCurrentStepType_MPGP"
PetscErrorCode QPSMPGPGetCurrentStepType_MPGP(QPS qps,char *stepType)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  *stepType = mpgp->currentStepType;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetAlpha_MPGP"
static PetscErrorCode QPSMPGPGetAlpha_MPGP(QPS qps,PetscReal *alpha,QPSScalarArgType *argtype)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  if (alpha) *alpha = mpgp->alpha_user;
  if (argtype) *argtype = mpgp->alpha_type;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetAlpha_MPGP"
static PetscErrorCode QPSMPGPSetAlpha_MPGP(QPS qps,PetscReal alpha,QPSScalarArgType argtype)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  mpgp->alpha_user = alpha;
  mpgp->alpha_type = argtype;
  qps->setupcalled = PETSC_FALSE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetGamma_MPGP"
static PetscErrorCode QPSMPGPGetGamma_MPGP(QPS qps,PetscReal *gamma)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  *gamma = mpgp->gamma;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetGamma_MPGP"
static PetscErrorCode QPSMPGPSetGamma_MPGP(QPS qps,PetscReal gamma)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  mpgp->gamma = gamma;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetOperatorMaxEigenvalue_MPGP"
static PetscErrorCode QPSMPGPGetOperatorMaxEigenvalue_MPGP(QPS qps,PetscReal *maxeig)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  *maxeig = mpgp->maxeig;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetOperatorMaxEigenvalue_MPGP"
static PetscErrorCode QPSMPGPSetOperatorMaxEigenvalue_MPGP(QPS qps,PetscReal maxeig)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  mpgp->maxeig = maxeig;
  qps->setupcalled = PETSC_FALSE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPUpdateMaxEigenvalue_MPGP"
static PetscErrorCode  QPSMPGPUpdateMaxEigenvalue_MPGP(QPS qps, PetscReal maxeig_update)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;
  PetscReal maxeig_old = mpgp->maxeig;
  PetscReal alpha_old = mpgp->alpha;

  PetscFunctionBegin;
  if (!qps->setupcalled) FLLOP_SETERRQ(PetscObjectComm((PetscObject)qps),PETSC_ERR_ARG_WRONGSTATE,"this routine is intended to be called after QPSSetUp");

  mpgp->maxeig = maxeig_old*maxeig_update;
  TRY( PetscInfo3(qps,"updating maxeig := %.8e = %.8e * %.8e = maxeig * maxeig_update\n",mpgp->maxeig,maxeig_old,maxeig_update) );

  if (mpgp->alpha_type == QPS_ARG_MULTIPLE) {
    mpgp->alpha = alpha_old/maxeig_update;
    TRY( PetscInfo3(qps,"updating alpha := %.8e = %.8e / %.8e = alpha / maxeig_update\n",mpgp->alpha,alpha_old,maxeig_update) );
  }

  //TODO temporary
  if (FllopDebugEnabled) {
    PetscReal lambda;
    TRY( MatGetMaxEigenvalue(qps->solQP->A,NULL,&lambda,mpgp->maxeig_tol,mpgp->maxeig_iter) );
    TRY( FllopDebug1("|maxeig_from_power_method - mpgp->maxeig| = %8e\n",PetscAbs(lambda-mpgp->maxeig)) );
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetOperatorMaxEigenvalueTolerance_MPGP"
static PetscErrorCode QPSMPGPGetOperatorMaxEigenvalueTolerance_MPGP(QPS qps,PetscReal *tol)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  *tol = mpgp->maxeig_tol;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetOperatorMaxEigenvalueTolerance_MPGP"
static PetscErrorCode QPSMPGPSetOperatorMaxEigenvalueTolerance_MPGP(QPS qps,PetscReal tol)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  mpgp->maxeig_tol = tol;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetOperatorMaxEigenvalueIterations_MPGP"
static PetscErrorCode QPSMPGPGetOperatorMaxEigenvalueIterations_MPGP(QPS qps,PetscInt *numit)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  *numit = mpgp->maxeig_iter;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetOperatorMaxEigenvalueIterations_MPGP"
static PetscErrorCode QPSMPGPSetOperatorMaxEigenvalueIterations_MPGP(QPS qps,PetscInt numit)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  mpgp->maxeig_iter = numit;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MPGPGrads"
/*
MPGPGrads - compute projected, chopped, and free gradient

Parameters:
+ qps - QP solver
- g - gradient
*/
static PetscErrorCode MPGPGrads(QPS qps, Vec x, Vec g)
{
  QP                qp;
  QPC               qpc;

  Vec               gP;                 /* ... projected gradient               */
  Vec               gr;                 /* ... reduced free gradient            */
  Vec               gc;                 /* ... chopped gradient                 */
  Vec               gf;                 /* ... free gradient                    */

  QPS_MPGP          *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  TRY( QPSGetSolvedQP(qps,&qp) );
  TRY( QPGetQPC(qp,&qpc) );

  gP                = qps->work[0];
  gr                = qps->work[6];
  gf                = qps->work[1];
  gc                = qps->work[2];

  TRY( QPCGrads(qpc,x,g,gf,gc) );
  TRY( QPCGradReduced(qpc,x,gf,mpgp->alpha,gr) );
  TRY( VecWAXPY(gP,1.0,gf,gc) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MPGPExpansionLength"
/*
MPGPExpansionLength - compute expanson step length type

Parameters:
. qps   - QP solver
*/
static PetscErrorCode MPGPExpansionLength(QPS qps)
{
  QP                qp;
  Mat               A;
  Vec               vecs[2];
  PetscReal         dots[2];
  QPS_MPGP          *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  TRY( QPSGetSolvedQP(qps,&qp) );
  TRY( QPGetOperator(qp, &A) );                   /* get hessian matrix */
  switch (mpgp->explengthtype) {
    case QPS_MPGP_EXPANSION_LENGTH_FIXED:
      break;
    case QPS_MPGP_EXPANSION_LENGTH_OPT:
      vecs[0] = qps->work[3]; /* g */
      vecs[1] = qps->work[5]; /* Ap  */
      TRY( MatMult(A,mpgp->expdirection,vecs[1]) );
      mpgp->nmv++;
      TRY( VecMDot(mpgp->explengthvec,2,vecs,dots) );
      mpgp->alpha = mpgp->alpha_user*dots[0]/dots[1];
      break;
    case QPS_MPGP_EXPANSION_LENGTH_OPTAPPROX:
      vecs[0] = qps->work[3]; /* g */
      vecs[1] = mpgp->explengthvec;
      TRY( VecMDot(mpgp->explengthvec,2,vecs,dots) );
      mpgp->alpha = mpgp->alpha_user*dots[0]/dots[1];
      mpgp->alpha = mpgp->alpha/mpgp->maxeig;
      break;
    case QPS_MPGP_EXPANSION_LENGTH_BB:
      vecs[0] = mpgp->explengthvecold;
      vecs[1] = qps->work[5]; /* Ap  */
      TRY( MatMult(A,mpgp->explengthvecold,vecs[1]) );
      mpgp->nmv++;
      TRY( VecMDot(mpgp->explengthvecold,2,vecs,dots) );
      mpgp->alpha = mpgp->alpha_user*dots[0]/dots[1];
      break;
    default: SETERRQ(PetscObjectComm((PetscObject)qps),PETSC_ERR_PLIB,"Unknown MPGP expansion length type");
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MPGPExpansion_Std"
/*
MPGPExpansion - expand active set

Parameters:
+ qps   - QP solver
. afeas - feasible step length in p direction
- acg   - cg step length in p direction
*/
static PetscErrorCode MPGPExpansion_Std(QPS qps, PetscReal afeas, PetscReal acg)
{
  QP                qp;
  Vec               g;                  /* ... gradient                         */
  Vec               p;                  /* ... conjugate gradient               */
  Vec               Ap;                 /* ... multiplicated vector             */
  Vec               x;
  QPS_MPGP          *mpgp = (QPS_MPGP*)qps->data;

  PetscFunctionBegin;
  TRY( QPSGetSolvedQP(qps,&qp) );
  TRY( QPGetSolutionVector(qp, &x) );
  g                 = qps->work[3];
  p                 = qps->work[4];
  Ap                = qps->work[5];

  /* make maximal feasible step */
  TRY( VecAXPY(x, -afeas, p) );             /* x=x-afeas*p*/
  TRY( VecAXPY(g, -afeas, Ap) );            /* g=g-afeas*Ap    */
  TRY( MPGPGrads(qps, x, g) );              /* grad. splitting  gP,gf,gc,gr */

  TRY( MPGPExpansionLength(qps) );
  TRY( VecAXPY(x, -mpgp->alpha, mpgp->expdirection) );      /* x=x-abar*direction */
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MPGPExpansion_ProjCG"
/*
MPGPExpansion - expand active set

Parameters:
+ qps   - QP solver
. afeas - feasible step length in p direction
- acg   - cg step length in p direction
*/
static PetscErrorCode MPGPExpansion_ProjCG(QPS qps, PetscReal afeas, PetscReal acg)
{
  QP                qp;
  Vec               p;                  /* ... conjugate gradient               */
  Vec               x;

  PetscFunctionBegin;
  TRY( QPSGetSolvedQP(qps,&qp) );
  TRY( QPGetSolutionVector(qp, &x) );
  p                 = qps->work[4];

  /* make projected CG step */
  TRY( VecAXPY(x, -acg, p) );               /* x=x-acg*p      */
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSSetup_MPGP"
/*
QPSSetup_MPGP - the setup function of MPGP algorithm; initialize constant step-size, check the constraints

Parameters:
. qps - QP solver
*/
PetscErrorCode QPSSetup_MPGP(QPS qps)
{
  QPS_MPGP          *mpgp = (QPS_MPGP*)qps->data;
  Vec               lb,ub;

  PetscFunctionBegin;
  /* set the number of working vectors */
  TRY( QPSSetWorkVecs(qps,7) );

  TRY( QPGetBox(qps->solQP,NULL,&lb,&ub) );
  if (mpgp->bchop_tol) {
    if (lb) TRY( VecChop(lb,mpgp->bchop_tol) );
    if (ub) TRY( VecChop(ub,mpgp->bchop_tol) );
  }

  switch (mpgp->exptype) {
    case QPS_MPGP_EXPANSION_STD:
      mpgp->expdirection = qps->work[6];     /* gr */
      mpgp->explengthvec = qps->work[6];
      if (mpgp->explengthtype == QPS_MPGP_EXPANSION_LENGTH_FIXED) {
        mpgp->expproject   = PETSC_FALSE;
      }
      break;
    case QPS_MPGP_EXPANSION_GF:
      mpgp->expdirection = qps->work[1];     /* gf */
      mpgp->explengthvec = qps->work[1];
      break;
    case QPS_MPGP_EXPANSION_G:
      mpgp->expdirection = qps->work[3];     /* g  */
      mpgp->explengthvec = qps->work[3];
      break;
    case QPS_MPGP_EXPANSION_GFGR:
      mpgp->expdirection = qps->work[1];     /* gf */
      mpgp->explengthvec = qps->work[6];
      break;
    case QPS_MPGP_EXPANSION_GGR:
      mpgp->expdirection = qps->work[3];     /* g  */
      mpgp->explengthvec = qps->work[6];
      break;
    case QPS_MPGP_EXPANSION_PROJCG:
      mpgp->expansion = MPGPExpansion_ProjCG;
      break;
    default: SETERRQ(PetscObjectComm((PetscObject)qps),PETSC_ERR_PLIB,"Unknown MPGP expansion type");
  }

  if (mpgp->explengthtype == QPS_MPGP_EXPANSION_LENGTH_BB) {
    TRY( VecDuplicate(mpgp->explengthvec,&mpgp->explengthvecold) );
  }

  /* initialize alpha */
  if (mpgp->alpha_type == QPS_ARG_MULTIPLE) {
    if (mpgp->maxeig == PETSC_DECIDE) {
      TRY( MatGetMaxEigenvalue(qps->solQP->A, NULL, &mpgp->maxeig, mpgp->maxeig_tol, mpgp->maxeig_iter) );
    }
    if (mpgp->alpha_user == PETSC_DECIDE) {
      mpgp->alpha_user = 2.0;
    }
    TRY( PetscInfo1(qps,"maxeig     = %.8e\n", mpgp->maxeig) );
    TRY( PetscInfo1(qps,"alpha_user = %.8e\n", mpgp->alpha_user) );
    mpgp->alpha = mpgp->alpha_user/mpgp->maxeig;
  } else {
    mpgp->alpha = mpgp->alpha_user;
  }
  TRY( PetscInfo1(qps,  "alpha      = %.8e\n", mpgp->alpha) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSSolve_MPGP"
/*
QPSSolve_MPGP - the solver; solve the problem using MPGP algorithm

Parameters:
+ qps - QP solver
*/
PetscErrorCode QPSSolve_MPGP(QPS qps)
{
  QPS_MPGP          *mpgp = (QPS_MPGP*)qps->data;
  QP                qp;
  QPC               qpc;
  Mat               A;                  /* ... hessian matrix                   */
  Vec               b;                  /* ... right-hand side vector           */
  Vec               x;                  /* ... vector of variables              */
  Vec               gP;                 /* ... projected gradient               */
  Vec               gc;                 /* ... chopped gradient                 */
  Vec               gf;                 /* ... free gradient                    */
  Vec               gr;                 /* ... reduced free gradient            */
  Vec               g;                  /* ... gradient                         */
  Vec               p;                  /* ... conjugate gradient               */
  Vec               Ap;                 /* ... multiplicated vector             */

  PetscReal         gamma2;             /* ... algorithm constants              */
  PetscReal         acg;                /* ... conjugate gradient step-size     */
  PetscReal         bcg;                /* ... cg ortogonalization parameter    */
  PetscReal         afeas;              /* ... maximum feasible step-size       */
  PetscReal         pAp, gcTgc, gfTgf;  /* ... results of dot products    */

  PetscInt          nmv=0;              /* ... matrix-vector mult. counter      */
  PetscInt          ncg=0;              /* ... cg step counter                  */
  PetscInt          nprop=0;            /* ... proportional step counter        */
  PetscInt          nexp=0;             /* ... expansion step counter           */

  PetscFunctionBegin;
  /* set working vectors */
  gP                = qps->work[0];
  gr                = qps->work[6];
  gf                = qps->work[1];
  gc                = qps->work[2];

  g                 = qps->work[3];
  p                 = qps->work[4];
  Ap                = qps->work[5];

  /* set constants of algorithm */
  gamma2            = mpgp->gamma*mpgp->gamma;

  TRY( QPSGetSolvedQP(qps,&qp) );
  TRY( QPGetQPC(qp,&qpc) );                       /* get constraints */
  TRY( QPGetSolutionVector(qp, &x) );             /* get the solution vector */
  TRY( QPGetOperator(qp, &A) );                   /* get hessian matrix */
  TRY( QPGetRhs(qp, &b) );                        /* get right-hand side vector */

  TRY( QPCProject(qpc,x,x) );                     /* project x initial guess to feasible set */

  /* compute gradient */
  TRY( MatMult(A, x, g) );                        /* g=A*x */
  nmv++;                                          /* matrix multiplication counter */
  TRY( VecAXPY(g, -1.0, b) );                     /* g=g-b */

  TRY( MPGPGrads(qps, x, g) );                    /* grad. splitting  gP,gf,gc */

  /* initiate CG method */
  TRY( VecCopy(gf, p) );                          /* p=gf */

  mpgp->currentStepType = ' ';
  qps->iteration = 0;                             /* main iteration counter */
  while (1)                                       /* main cycle */
  {
    /* compute the norm of projected gradient - stopping criterion */
    TRY( VecNorm(gP, NORM_2, &qps->rnorm) );      /* qps->rnorm=norm(gP)*/

    /* compute dot products to control the proportionality */
    TRY( VecDot(gc, gc, &gcTgc) );               /* gcTgc=gc'*gc   */
    TRY( VecDot(gr, gf, &gfTgf) );               /* gfTgf=gr'*gf   */

    /* compute norm of gf, gc from computed dot products */
    if (qps->numbermonitors) {
      mpgp->gfnorm =  PetscSqrtScalar(gfTgf);
      mpgp->gcnorm =  PetscSqrtScalar(gcTgc);
      TRY( QPSMonitor(qps,qps->iteration,qps->rnorm)) ;
    }

    /* test the convergence of algorithm */
    TRY( (*qps->convergencetest)(qps,&qps->reason) ); /* test for convergence */
    if (qps->reason != KSP_CONVERGED_ITERATING) break;

    /* save old direction vec for BB expansion step length */
    if (mpgp->explengthtype == QPS_MPGP_EXPANSION_LENGTH_BB) {
      TRY( VecCopy(mpgp->explengthvec,mpgp->explengthvecold) );
    }

    /* proportional condition */
    if (gcTgc <= gamma2*gfTgf)                    /* u is proportional */
    {
      TRY( MatMult(A, p, Ap) );                   /* Ap=A*p */
      nmv++;                                      /* matrix multiplication counter */

      /* compute step-sizes */
      TRY( VecDot(p, Ap, &pAp) );                 /* pAp=p'*Ap      */
      TRY( VecDot(g,  p, &acg) );                 /* acg=g'*p       */
      acg  = acg/pAp;                             /* acg=acg/pAp    */
      TRY( QPCFeas(qpc, x, p, &afeas) );          /* finds max.feas.steplength */

      /* decide if it is able to do full CG step */
      if (acg <= afeas)
      {
        /* CONJUGATE GRADIENT STEP */
        ncg++;                                    /* increase CG step counter */
        mpgp->currentStepType = 'c';

        /* make CG step */
        TRY( VecAXPY(x, -acg, p) );               /* x=x-acg*p      */
        TRY( VecAXPY(g, -acg, Ap) );              /* g=g-acg*Ap      */

        TRY( MPGPGrads(qps, x, g) );              /* grad. splitting  gP,gf,gc */

        /* compute orthogonalization parameter and next orthogonal vector */
        TRY( VecDot(Ap, gf, &bcg) );              /* bcg=Ap'*gf     */
        bcg  = bcg/pAp;                           /* bcg=bcg/pAp     */
        TRY( VecAYPX(p, -bcg, gf) );              /* p=gf-bcg*p     */
      }
      else                                        /* expansion step  */
      {
        /* EXPANSION STEP */
        mpgp->expansion(qps,afeas,acg);
        if (mpgp->expproject) {
          TRY( QPCProject(qpc, x, x) );             /* project x to feas.set */
        }

        /* compute new gradient */
        TRY( MatMult(A, x, g) );                  /* g=A*x */
        nmv++;                                    /* matrix multiplication counter */
        TRY( VecAXPY(g, -1.0, b) );               /* g=g-b           */

        TRY( MPGPGrads(qps, x, g) );              /* grad. splitting  gP,gf,gc */

        /* restart CG method */
        TRY( VecCopy(gf, p) );                    /* p=gf           */

        nexp++;                                   /* increase expansion step counter */
        mpgp->currentStepType = 'e';
      }
    }
    else                                          /* proportioning step  */
    {
      /* PROPORTIONING STEP */
      nprop++;                                    /* increase proportioning step counter */
      mpgp->currentStepType = 'p';

      TRY( VecCopy(gc, p) );                      /* p=gc           */
      TRY( MatMult(A, p, Ap) );                   /* Ap=A*p */
      nmv++;                                      /* matrix multiplication counter */

      /* compute step-size */
      TRY( VecDot(p, Ap, &pAp) );                 /* pAp=p'*Ap       */
      TRY( VecDot(g,  p, &acg) );                 /* acg=g'*p        */
      acg  = acg/pAp;                             /* acg=acg/pAp     */

      /* make a step */
      TRY( VecAXPY(x, -acg, p) );                 /* x=x-acg*p       */
      TRY( VecAXPY(g, -acg, Ap) );                /* g=g-acg*Ap      */

      TRY( MPGPGrads(qps, x, g) );                /* grad. splitting  gP,gf,gc */

      /* restart CG method */
      TRY( VecCopy(gf, p) );                      /* p=gf           */
    }
    qps->iteration++;
  };

  mpgp->ncg     += ncg;
  mpgp->nexp    += nexp;
  mpgp->nmv     += nmv;
  mpgp->nprop   += nprop;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSResetStatistics_MPGP"
PetscErrorCode QPSResetStatistics_MPGP(QPS qps)
{
  QPS_MPGP *mpgp = (QPS_MPGP*)qps->data;
  PetscFunctionBegin;
  mpgp->ncg   = 0;
  mpgp->nexp  = 0;
  mpgp->nmv   = 0;
  mpgp->nprop = 0;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSDestroy_MPGP"
/*
QPSDestroy_MPGP - MPGP afterparty

Parameters:
. qps - QP solver
*/
PetscErrorCode QPSDestroy_MPGP(QPS qps)
{
  PetscFunctionBegin;
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetAlpha_MPGP_C",NULL) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetAlpha_MPGP_C",NULL) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetGamma_MPGP_C",NULL) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetGamma_MPGP_C",NULL) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetOperatorMaxEigenvalue_MPGP_C",NULL) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetOperatorMaxEigenvalue_MPGP_C",NULL) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetOperatorMaxEigenvalueTolerance_MPGP_C",NULL) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetOperatorMaxEigenvalueTolerance_MPGP_C",NULL) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetOperatorMaxEigenvalueIterations_MPGP_C",NULL) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetOperatorMaxEigenvalueIterations_MPGP_C",NULL) );
  TRY( QPSDestroyDefault(qps) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSIsQPCompatible_MPGP"
PetscErrorCode QPSIsQPCompatible_MPGP(QPS qps,QP qp,PetscBool *flg)
{
  Mat Beq,Bineq;
  Vec ceq,cineq;
  QPC qpc;

  PetscFunctionBegin;
  TRY( QPGetEq(qp,&Beq,&ceq) );
  TRY( QPGetIneq(qp,&Bineq,&cineq) );
  TRY( QPGetQPC(qp,&qpc) );
  if (Beq || ceq || Bineq || cineq) {
    *flg = PETSC_FALSE;
  } else {
    TRY( PetscObjectTypeCompare((PetscObject)qpc,QPCBOX,flg) );
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSSetFromOptions_MPGP"
PetscErrorCode QPSSetFromOptions_MPGP(PetscOptionItems *PetscOptionsObject,QPS qps)
{
  QPS_MPGP    *mpgp = (QPS_MPGP*)qps->data;
  PetscBool flg1,flg2,alpha_direct;
  PetscReal maxeig,maxeig_tol,alpha,gamma;
  PetscInt maxeig_iter;

  PetscFunctionBegin;
  TRY( PetscOptionsHead(PetscOptionsObject,"QPS MPGP options") );

  alpha_direct = PETSC_FALSE;
  TRY( PetscOptionsBool("-qps_mpgp_alpha_direct","","QPSMPGPSetAlpha",(PetscBool) mpgp->alpha_type,&alpha_direct,&flg1) );
  TRY( PetscOptionsReal("-qps_mpgp_alpha","","QPSMPGPSetAlpha",mpgp->alpha_user,&alpha,&flg2) );
  if (flg1 || flg2) TRY( QPSMPGPSetAlpha(qps,alpha,(QPSScalarArgType) alpha_direct) );
  TRY( PetscOptionsReal("-qps_mpgp_gamma","","QPSMPGPSetGamma",mpgp->gamma,&gamma,&flg1) );
  if (flg1) TRY( QPSMPGPSetGamma(qps,gamma) );
  TRY( PetscOptionsReal("-qps_mpgp_maxeig","Approximate maximum eigenvalue of the Hessian, PETSC_DECIDE means this is automatically computed.","QPSMPGPSetOperatorMaxEigenvalue",mpgp->maxeig,&maxeig,&flg1) );
  if (flg1) TRY( QPSMPGPSetOperatorMaxEigenvalue(qps,maxeig) );
  TRY( PetscOptionsReal("-qps_mpgp_maxeig_tol","Relative tolerance to find approximate maximum eigenvalue of the Hessian, PETSC_DECIDE means QPS rtol","QPSMPGPSetOperatorMaxEigenvalueTolerance",mpgp->maxeig_tol,&maxeig_tol,&flg1) );
  if (flg1) TRY( QPSMPGPSetOperatorMaxEigenvalueTolerance(qps,maxeig_tol) );
  TRY( PetscOptionsInt("-qps_mpgp_maxeig_iter","Number of iterations to find an approximate maximum eigenvalue of the Hessian","QPSMPGPSetOperatorMaxEigenvalueIterations",mpgp->maxeig_iter,&maxeig_iter,&flg1) );
  if (flg1) TRY( QPSMPGPSetOperatorMaxEigenvalueIterations(qps,maxeig_iter) );
  TRY( PetscOptionsReal("-qps_mpgp_btol","Boundary overshoot tolerance; default: 10*PETSC_MACHINE_EPSILON","",mpgp->btol,&mpgp->btol,&flg1) );
  TRY( PetscOptionsReal("-qps_mpgp_bound_chop_tol","Sets boundary to 0 for |boundary|<tol ; default: 0","",mpgp->bchop_tol,&mpgp->bchop_tol,NULL) );
  TRY( PetscOptionsEnum("-qps_mpgp_expansion_type","Set expansion step type","",QPSMPGPExpansionTypes,(PetscEnum)mpgp->exptype,(PetscEnum*)&mpgp->exptype,NULL) );
  TRY( PetscOptionsEnum("-qps_mpgp_expansion_length_type","Set expansion step length type","",QPSMPGPExpansionLengthTypes,(PetscEnum)mpgp->explengthtype,(PetscEnum*)&mpgp->explengthtype,NULL) );
  TRY( PetscOptionsTail() );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSViewConvergence_MPGP"
PetscErrorCode QPSViewConvergence_MPGP(QPS qps, PetscViewer v)
{
  QPS_MPGP      *mpgp = (QPS_MPGP*)qps->data;
  PetscBool     iascii;

  PetscFunctionBegin;
  TRY( PetscObjectTypeCompare((PetscObject)v,PETSCVIEWERASCII,&iascii) );
  if (iascii) {
    TRY( PetscViewerASCIIPrintf(v,"from the last QPSReset:\n") );
    TRY( PetscViewerASCIIPrintf(v,"number of Hessian multiplications %d\n",mpgp->nmv) );
    TRY( PetscViewerASCIIPrintf(v,"number of CG steps %d\n",mpgp->ncg) );
    TRY( PetscViewerASCIIPrintf(v,"number of expansion steps %d\n",mpgp->nexp) );
    TRY( PetscViewerASCIIPrintf(v,"number of proportioning steps %d\n",mpgp->nprop) );
  }
  PetscFunctionReturn(0);
}
#undef __FUNCT__
#define __FUNCT__ "QPSCreate_MPGP"
FLLOP_EXTERN PetscErrorCode QPSCreate_MPGP(QPS qps)
{
  QPS_MPGP         *mpgp;

  PetscFunctionBegin;
  TRY( PetscNewLog(qps,&mpgp) );
  qps->data                  = (void*)mpgp;

  mpgp->alpha_user           = PETSC_DECIDE;
  mpgp->alpha_type           = QPS_ARG_MULTIPLE;
  mpgp->gamma                = 1.0;
  mpgp->maxeig               = PETSC_DECIDE;
  mpgp->maxeig_tol           = PETSC_DECIDE;
  mpgp->maxeig_iter          = PETSC_DECIDE;
  mpgp->btol                 = 10*PETSC_MACHINE_EPSILON; /* boundary tol */
  mpgp->bchop_tol            = 0.0; /* chop of bounds */

  mpgp->exptype              = QPS_MPGP_EXPANSION_STD;
  mpgp->explengthtype        = QPS_MPGP_EXPANSION_LENGTH_FIXED;
  mpgp->expansion            = MPGPExpansion_Std;
  mpgp->expproject           = PETSC_TRUE;

  /*
       Sets the functions that are associated with this data structure
       (in C++ this is the same as defining virtual functions)
  */
  qps->ops->setup            = QPSSetup_MPGP;
  qps->ops->solve            = QPSSolve_MPGP;
  qps->ops->resetstatistics  = QPSResetStatistics_MPGP;
  qps->ops->destroy          = QPSDestroy_MPGP;
  qps->ops->isqpcompatible   = QPSIsQPCompatible_MPGP;
  qps->ops->setfromoptions   = QPSSetFromOptions_MPGP;
  qps->ops->monitor          = QPSMonitorDefault_MPGP;
  qps->ops->viewconvergence  = QPSViewConvergence_MPGP;

  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetCurrentStepType_MPGP_C",QPSMPGPGetCurrentStepType_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetAlpha_MPGP_C",QPSMPGPGetAlpha_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetAlpha_MPGP_C",QPSMPGPSetAlpha_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetGamma_MPGP_C",QPSMPGPGetGamma_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetGamma_MPGP_C",QPSMPGPSetGamma_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetOperatorMaxEigenvalue_MPGP_C",QPSMPGPGetOperatorMaxEigenvalue_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetOperatorMaxEigenvalue_MPGP_C",QPSMPGPSetOperatorMaxEigenvalue_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetOperatorMaxEigenvalueTolerance_MPGP_C",QPSMPGPSetOperatorMaxEigenvalueTolerance_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetOperatorMaxEigenvalueTolerance_MPGP_C",QPSMPGPGetOperatorMaxEigenvalueTolerance_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPGetOperatorMaxEigenvalueIterations_MPGP_C",QPSMPGPGetOperatorMaxEigenvalueIterations_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPSetOperatorMaxEigenvalueIterations_MPGP_C",QPSMPGPSetOperatorMaxEigenvalueIterations_MPGP) );
  TRY( PetscObjectComposeFunction((PetscObject)qps,"QPSMPGPUpdateMaxEigenvalue_MPGP_C",QPSMPGPUpdateMaxEigenvalue_MPGP) );
  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetCurrentStepType"
PetscErrorCode QPSMPGPGetCurrentStepType(QPS qps,char *stepType)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  if (stepType) PetscValidRealPointer(stepType,2);
  *stepType = ' ';
  TRY( PetscTryMethod(qps,"QPSMPGPGetCurrentStepType_MPGP_C",(QPS,char*),(qps,stepType)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetAlpha"
/*@
QPSMPGPGetAlpha - get the constant step-size used in algorithm based on spectral properties of Hessian matrix

Parameters:
+ qps - QP solver
. alpha - pointer to store the value
- argtype -

Level: advanced
@*/
PetscErrorCode QPSMPGPGetAlpha(QPS qps,PetscReal *alpha,QPSScalarArgType *argtype)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  if (alpha) PetscValidPointer(alpha,2);
  if (argtype) PetscValidPointer(argtype,3);
  TRY( PetscUseMethod(qps,"QPSMPGPGetAlpha_MPGP_C",(QPS,PetscReal*,QPSScalarArgType*),(qps,alpha,argtype)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetAlpha"
/*@
QPSMPGPSetAlpha - set the constant step-size used in algorithm based on spectral properties of Hessian matrix

Parameters:
+ qps - QP solver
. alpha - new value of parameter
- argtype -

Level: intermediate
@*/
PetscErrorCode QPSMPGPSetAlpha(QPS qps,PetscReal alpha,QPSScalarArgType argtype)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidLogicalCollectiveReal(qps,alpha,2);
  TRY( PetscTryMethod(qps,"QPSMPGPSetAlpha_MPGP_C",(QPS,PetscReal,QPSScalarArgType),(qps,alpha,argtype)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetGamma"
/*@
QPSMPGPGetGamma - get the proportioning parameter used in algorithm

Parameters:
+ qps - QP solver
- gamma - pointer to store the value

Level: advanced
@*/
PetscErrorCode QPSMPGPGetGamma(QPS qps,PetscReal *gamma)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidPointer(gamma,2);
  TRY( PetscUseMethod(qps,"QPSMPGPGetGamma_MPGP_C",(QPS,PetscReal*),(qps,gamma)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetGamma"
/*@
QPSMPGPSetGamma - set the proportioning parameter used in algorithm

Parameters:
+ qps - QP solver
- gamma - new value of parameter

Level: intermediate
@*/
PetscErrorCode QPSMPGPSetGamma(QPS qps,PetscReal gamma)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidLogicalCollectiveReal(qps,gamma,2);
  TRY( PetscTryMethod(qps,"QPSMPGPSetGamma_MPGP_C",(QPS,PetscReal),(qps,gamma)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetOperatorMaxEigenvalue"
PetscErrorCode QPSMPGPGetOperatorMaxEigenvalue(QPS qps,PetscReal *maxeig)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidPointer(maxeig,2);
  TRY( PetscUseMethod(qps,"QPSMPGPGetOperatorMaxEigenvalue_MPGP_C",(QPS,PetscReal*),(qps,maxeig)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetOperatorMaxEigenvalue"
/*@
QPSMPGPSetOperatorMaxEigenvalue - set the estimation of largest eigenvalue

Parameters:
+ qps - QP solver
- maxeig - new value

Level: intermediate
@*/
PetscErrorCode QPSMPGPSetOperatorMaxEigenvalue(QPS qps,PetscReal maxeig)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidLogicalCollectiveReal(qps,maxeig,2);
  if (maxeig < 0 && maxeig != PETSC_DECIDE) FLLOP_SETERRQ(((PetscObject)qps)->comm,PETSC_ERR_ARG_OUTOFRANGE,"Argument must be nonnegative");
  TRY( PetscTryMethod(qps,"QPSMPGPSetOperatorMaxEigenvalue_MPGP_C",(QPS,PetscReal),(qps,maxeig)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPUpdateMaxEigenvalue"
PetscErrorCode  QPSMPGPUpdateMaxEigenvalue(QPS qps, PetscReal maxeig_update)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidLogicalCollectiveReal(qps,maxeig_update,2);
  if (maxeig_update == 1.0) PetscFunctionReturn(0);
  TRY( PetscTryMethod(qps,"QPSMPGPUpdateMaxEigenvalue_MPGP_C",(QPS,PetscReal),(qps,maxeig_update)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetOperatorMaxEigenvalueTolerance"
/*@
QPSMPGPSetOperatorMaxEigenvalueTolerance - set the tolerance of the largest eigenvalue computation

Parameters:
+ qps - QP solver
- tol - new value

Level: intermediate
@*/
PetscErrorCode QPSMPGPSetOperatorMaxEigenvalueTolerance(QPS qps,PetscReal tol)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidLogicalCollectiveReal(qps,tol,2);
  TRY( PetscTryMethod(qps,"QPSMPGPSetOperatorMaxEigenvalueTolerance_MPGP_C",(QPS,PetscReal),(qps,tol)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetOperatorMaxEigenvalueTolerance"
/*@
QPSMPGPGetOperatorMaxEigenvalueTolerance - get the tolerance of the largest eigenvalue computation

Parameters:
+ qps - QP solver
- tol - pointer to returned value

Level: advanced
@*/
PetscErrorCode QPSMPGPGetOperatorMaxEigenvalueTolerance(QPS qps,PetscReal *tol)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidPointer(tol,2);
  TRY( PetscTryMethod(qps,"QPSMPGPGetOperatorMaxEigenvalueTolerance_MPGP_C",(QPS,PetscReal*),(qps,tol)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPGetOperatorMaxEigenvalueIterations"
/*@
QPSMPGPGetOperatorMaxEigenvalueIterations - get the maximum number of iterations to obtain the largest eigenvalue computation

Parameters:
+ qps - QP solver
- numit - pointer to returned value

Level: advanced
@*/
PetscErrorCode QPSMPGPGetOperatorMaxEigenvalueIterations(QPS qps,PetscInt *numit)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidPointer(numit,2);
  TRY( PetscUseMethod(qps,"QPSMPGPGetOperatorMaxEigenvalueIterations_MPGP_C",(QPS,PetscInt*),(qps,numit)) );
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "QPSMPGPSetOperatorMaxEigenvalueIterations"
/*@
QPSMPGPSetOperatorMaxEigenvalueIterations - set the maximum number of iterations to obtain the largest eigenvalue computation

Parameters:
+ qps - QP solver
- numit - new value

Level: intermediate
@*/
PetscErrorCode QPSMPGPSetOperatorMaxEigenvalueIterations(QPS qps,PetscInt numit)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(qps,QPS_CLASSID,1);
  PetscValidLogicalCollectiveInt(qps,numit,2);
  if (numit <= 1) FLLOP_SETERRQ(((PetscObject)qps)->comm,PETSC_ERR_ARG_OUTOFRANGE,"Argument must be > 1");
  TRY( PetscTryMethod(qps,"QPSMPGPSetOperatorMaxEigenvalueIterations_MPGP_C",(QPS,PetscInt),(qps,numit)) );
  PetscFunctionReturn(0);
}

