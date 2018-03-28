#include "dcgimpl.h"

#if defined(HAVE_SLEPC)
#include <slepceps.h>
#include <slepcbv.h>
#endif

const char *const KSPDCGSpaceTypes[] = {
  "haar",
  "jacket-haar",
  "db4",
  "db8",
  "db16",
  "biorth22",
  "meyer",
  "aggregation",
  "slepc",
  "slepc-cheap",
  "user",
  "DCGSpaceType",
  "DCG_SPACE_",
  0
};

//dec low high
PetscScalar db4[] = {-0.12940952255092145,0.22414386804185735,0.836516303737469,0.48296291314469025};
//rec low high
//PetscScalar db4[] = {0.48296291314469025,0.836516303737469,0.22414386804185735,-0.12940952255092145};

PetscScalar db8[] = {-0.010597401784997278,
0.032883011666982945,
0.030841381835986965,
-0.18703481171888114,
-0.02798376941698385,
0.6308807679295904,
0.7148465705525415,
0.23037781330885523};

//PetscScalar db8[] = {0.23037781330885523,
//0.7148465705525415,
//0.6308807679295904,
//-0.02798376941698385,
//-0.18703481171888114,
//0.030841381835986965,
//0.032883011666982945,
//-0.010597401784997278};

PetscScalar db16[] = {-0.00011747678400228192,
0.0006754494059985568,
-0.0003917403729959771,
-0.00487035299301066,
0.008746094047015655,
0.013981027917015516,
-0.04408825393106472,
-0.01736930100202211,
0.128747426620186,
0.00047248457399797254,
-0.2840155429624281,
-0.015829105256023893,
0.5853546836548691,
0.6756307362980128,
0.3128715909144659,
0.05441584224308161};

//PetscScalar db16[] = {0.05441584224308161,
//0.3128715909144659,
//0.6756307362980128,
//0.5853546836548691,
//-0.015829105256023893,
//-0.2840155429624281,
//0.00047248457399797254,
//0.128747426620186,
//-0.01736930100202211,
//-0.04408825393106472,
//0.013981027917015516,
//0.008746094047015655,
//-0.00487035299301066,
//-0.0003917403729959771,
//0.0006754494059985568,
//-0.00011747678400228192};

PetscScalar biorth22[] = {0.0,
-0.1767766952966369,
0.3535533905932738,
1.0606601717798214,
0.3535533905932738,
-0.1767766952966369};

//PetscScalar biorth22[] = {0.0,
//0.3535533905932738,
//0.7071067811865476,
//0.3535533905932738,
//0.0,
//0.0};

PetscReal meyer[] = {0.0,-1.009999956941423e-12,8.519459636796214e-09,-1.111944952595278e-08,-1.0798819539621958e-08,6.066975741351135e-08,-1.0866516536735883e-07,8.200680650386481e-08,1.1783004497663934e-07,-5.506340565252278e-07,1.1307947017916706e-06,-1.489549216497156e-06,7.367572885903746e-07,3.20544191334478e-06,-1.6312699734552807e-05,6.554305930575149e-05,-0.0006011502343516092,-0.002704672124643725,0.002202534100911002,0.006045814097323304,-0.006387718318497156,-0.011061496392513451,0.015270015130934803,0.017423434103729693,-0.03213079399021176,-0.024348745906078023,0.0637390243228016,0.030655091960824263,-0.13284520043622938,-0.035087555656258346,0.44459300275757724,0.7445855923188063,0.44459300275757724,-0.035087555656258346,-0.13284520043622938,0.030655091960824263,0.0637390243228016,-0.024348745906078023,-0.03213079399021176,0.017423434103729693,0.015270015130934803,-0.011061496392513451,-0.006387718318497156,0.006045814097323304,0.002202534100911002,-0.002704672124643725,-0.0006011502343516092,6.554305930575149e-05,-1.6312699734552807e-05,3.20544191334478e-06,7.367572885903746e-07,-1.489549216497156e-06,1.1307947017916706e-06,-5.506340565252278e-07,1.1783004497663934e-07,8.200680650386481e-08,-1.0866516536735883e-07,6.066975741351135e-08,-1.0798819539621958e-08,-1.111944952595278e-08,8.519459636796214e-09,-1.009999956941423e-12};

#undef __FUNCT__
#define __FUNCT__ "KSPDCGDeflationSpaceCreateJacketHaar"
static PetscErrorCode KSPDCGDeflationSpaceCreateJacketHaar(MPI_Comm comm,PetscInt m,PetscInt n,PetscInt M,PetscInt N,PetscBool jacket,Mat *H)
{
  PetscErrorCode ierr;
  Mat defl;
  PetscInt i,j,ilo,ihi,alloc=2,*Iidx;
  PetscReal val,*row;

  PetscFunctionBegin;
  if (jacket) alloc = 3;
  ierr = PetscMalloc1(alloc,&row);CHKERRQ(ierr);
  ierr = PetscMalloc1(alloc,&Iidx);CHKERRQ(ierr);

  val = 1./pow(2,0.5);
  row[0] = val;
  row[1] = val;

  /* TODO pass A instead of KSP? */
  ierr = MatCreate(comm,&defl);CHKERRQ(ierr);
  ierr = MatSetSizes(defl,m,n,M,N);CHKERRQ(ierr);
  ierr = MatSetUp(defl);CHKERRQ(ierr);
  ierr = MatSeqAIJSetPreallocation(defl,alloc,NULL);CHKERRQ(ierr);
  ierr = MatMPIAIJSetPreallocation(defl,alloc,NULL,alloc,NULL);CHKERRQ(ierr);
  ierr = MatSetOption(defl,MAT_NEW_NONZERO_ALLOCATION_ERR,PETSC_TRUE);CHKERRQ(ierr);
  ierr = MatSetOption(defl,MAT_IGNORE_OFF_PROC_ENTRIES,PETSC_TRUE);CHKERRQ(ierr);
  
  ierr = MatGetOwnershipRange(defl,&ilo,&ihi);CHKERRQ(ierr);
  for (i=0; i<2; i++) Iidx[i] = i+ilo*2;
  if (jacket && ihi==M) ihi -=2;
  if (ihi<ilo) SETERRQ1(comm,PETSC_ERR_ARG_WRONG,"To many cores to assemble Jacket Haar matrix with %d rows",M);
  for (i=ilo; i<ihi; i++) {
    ierr = MatSetValues(defl,1,&i,2,Iidx,row,INSERT_VALUES);CHKERRQ(ierr);
    for (j=0; j<2; j++) Iidx[j] += 2;
  }
  if (jacket && ihi == M-2) {
    for (i=0; i<3; i++) Iidx[i] = i+ilo*2;
    row[0] = 0.5; row[1] = 0.5; row[2] = val;
    ierr = MatSetValues(defl,1,&ihi,3,Iidx,row,INSERT_VALUES);CHKERRQ(ierr);
    ihi += 1;
    row[2] = -row[2];
    ierr = MatSetValues(defl,1,&ihi,3,Iidx,row,INSERT_VALUES);CHKERRQ(ierr);
  }
    
  ierr = MatAssemblyBegin(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  
  ierr = PetscFree(row);CHKERRQ(ierr);
  ierr = PetscFree(Iidx);CHKERRQ(ierr);
  *H = defl;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "KSPDCGDeflationSpaceCreateWave"
static PetscErrorCode KSPDCGDeflationSpaceCreateWave(MPI_Comm comm,PetscInt m,PetscInt n,PetscInt M,PetscInt N,PetscInt ncoeffs,PetscScalar *coeffs,PetscBool trunc,Mat *H)
{
  PetscErrorCode ierr;
  Mat defl;
  PetscInt i,j,k,ilo,ihi,*Iidx;

  PetscFunctionBegin;
  ierr = PetscMalloc1(ncoeffs,&Iidx);CHKERRQ(ierr);

  /* TODO pass A instead of KSP? */
  ierr = MatCreate(comm,&defl);CHKERRQ(ierr);
  ierr = MatSetSizes(defl,m,n,M,N);CHKERRQ(ierr);
  ierr = MatSetUp(defl);CHKERRQ(ierr);
  ierr = MatSeqAIJSetPreallocation(defl,ncoeffs,NULL);CHKERRQ(ierr);
  ierr = MatMPIAIJSetPreallocation(defl,ncoeffs,NULL,ncoeffs,NULL);CHKERRQ(ierr);
  ierr = MatSetOption(defl,MAT_NEW_NONZERO_ALLOCATION_ERR,PETSC_TRUE);CHKERRQ(ierr);
  ierr = MatSetOption(defl,MAT_IGNORE_OFF_PROC_ENTRIES,PETSC_TRUE);CHKERRQ(ierr);

  /* Alg 735 Taswell: fvecmat */
  k = ncoeffs -2;
  if (trunc) k = k/2;
  
  ierr = MatGetOwnershipRange(defl,&ilo,&ihi);CHKERRQ(ierr);
  for (i=0; i<ncoeffs; i++) {
    Iidx[i] = i+ilo*2 -k;
    if (Iidx[i] >= N) Iidx[i] = PETSC_MIN_INT;
  }
  for (i=ilo; i<ihi; i++) {
    ierr = MatSetValues(defl,1,&i,ncoeffs,Iidx,coeffs,INSERT_VALUES);CHKERRQ(ierr);
    for (j=0; j<ncoeffs; j++) {
      Iidx[j] += 2;
      if (Iidx[j] >= N) Iidx[j] = PETSC_MIN_INT;
    }
  }
    
  ierr = MatAssemblyBegin(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  
  ierr = PetscFree(Iidx);CHKERRQ(ierr);
  *H = defl;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "KSPDCGGetDeflationSpaceHaar"
PetscErrorCode KSPDCGGetDeflationSpaceHaar(KSP ksp,Mat *W,PetscInt size)
{
  PetscErrorCode ierr;
  Mat A,defl;
  PetscInt i,j,len,ilo,ihi,*Iidx,m,M;
  PetscReal *col,val;

  PetscFunctionBegin;
  /* Haar basis wavelet, level=size */
  len = pow(2,size);
  ierr = PetscMalloc1(len,&col);CHKERRQ(ierr);
  ierr = PetscMalloc1(len,&Iidx);CHKERRQ(ierr);
  val = 1./pow(2,size/2.);
  for (i=0; i<len; i++) col[i] = val;

  /* TODO pass A instead of KSP? */
  ierr = KSPGetOperators(ksp,&A,NULL);CHKERRQ(ierr); /* NOTE: Get Pmat instead? */
  ierr = MatGetLocalSize(A,&m,NULL);CHKERRQ(ierr);
  ierr = MatGetSize(A,&M,NULL);CHKERRQ(ierr);
  ierr = MatCreate(PetscObjectComm((PetscObject)A),&defl);CHKERRQ(ierr);
  ierr = MatSetSizes(defl,m,PETSC_DECIDE,M,ceil(M/(float)len));CHKERRQ(ierr);
  ierr = MatSetUp(defl);CHKERRQ(ierr);
  ierr = MatSeqAIJSetPreallocation(defl,size,NULL);CHKERRQ(ierr);
  ierr = MatMPIAIJSetPreallocation(defl,size,NULL,size,NULL);CHKERRQ(ierr);
  ierr = MatSetOption(defl,MAT_NEW_NONZERO_ALLOCATION_ERR,PETSC_TRUE);CHKERRQ(ierr);
  
  ierr = MatGetOwnershipRangeColumn(defl,&ilo,&ihi);CHKERRQ(ierr);
  for (i=0; i<len; i++) Iidx[i] = i+ilo*len;
  if (M%len && ihi == (int)ceil(M/(float)len)) ihi -= 1;
  for (i=ilo; i<ihi; i++) {
    ierr = MatSetValues(defl,len,Iidx,1,&i,col,INSERT_VALUES);CHKERRQ(ierr);
    for (j=0; j<len; j++) Iidx[j] += len;
  }
  if (M%len && ihi+1 == ceil(M/(float)len)) {
    len = M%len;
    val = 1./pow(pow(2,len),0.5);
    for (i=0; i<len; i++) col[i] = val;
    ierr = MatSetValues(defl,len,Iidx,1,&ihi,col,INSERT_VALUES);CHKERRQ(ierr);
  }
    
  ierr = MatAssemblyBegin(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  
  ierr = PetscFree(col);CHKERRQ(ierr);
  ierr = PetscFree(Iidx);CHKERRQ(ierr);
  *W = defl;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "KSPDCGGetDeflationSpacetHaarOrig"
PetscErrorCode KSPDCGGetDeflationSpacetHaarOrig(KSP ksp,Mat *W,PetscInt size)
{
  PetscErrorCode ierr;
  Mat A,defl;
  PetscInt i,j,len,ilo,ihi,*Iidx,m,M;
  PetscReal *col,val;

  PetscFunctionBegin;
  /* Haar basis wavelet, level=size */
  len = pow(2,size);
  ierr = PetscMalloc1(len,&col);CHKERRQ(ierr);
  ierr = PetscMalloc1(len,&Iidx);CHKERRQ(ierr);
  val = 1./pow(2,size/2.);
  /* mat G
  col[0] = val;
  col[1] = -val;
  while (2*start <= len) {
    for (i=start; i<2*start; i++) {
      col[i] = -col[i-start];
    }
    start = 2*start;
  }
  */
  for (i=0; i<len; i++) {
      col[i] = val;
  }

  /* TODO pass A instead of KSP? */
  ierr = KSPGetOperators(ksp,&A,NULL);CHKERRQ(ierr); /* NOTE: Get Pmat instead? */
  ierr = MatGetLocalSize(A,&m,NULL);CHKERRQ(ierr);
  ierr = MatGetSize(A,&M,NULL);CHKERRQ(ierr);
  ierr = MatCreate(PetscObjectComm((PetscObject)A),&defl);CHKERRQ(ierr);
  ierr = MatSetSizes(defl,m,PETSC_DECIDE,M,M/len);CHKERRQ(ierr);
  ierr = MatSetUp(defl);CHKERRQ(ierr);
  ierr = MatSeqAIJSetPreallocation(defl,size,NULL);CHKERRQ(ierr);
  ierr = MatMPIAIJSetPreallocation(defl,size,NULL,size,NULL);CHKERRQ(ierr);
  ierr = MatSetOption(defl,MAT_NEW_NONZERO_ALLOCATION_ERR,PETSC_TRUE);CHKERRQ(ierr);
  
  /* TODO change to transpose assembly, add MAT_IGNORE_OFF_PROC_ENTRIES */
  ierr = MatGetOwnershipRangeColumn(defl,&ilo,&ihi);CHKERRQ(ierr);
  for (i=0; i<len; i++) Iidx[i] = i+ilo*len;
  for (i=ilo; i<ihi; i++) {
    ierr = MatSetValues(defl,len,Iidx,1,&i,col,INSERT_VALUES);CHKERRQ(ierr);
    for (j=0; j<len; j++) Iidx[j] += len;
  }
  ierr = MatAssemblyBegin(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  
  ierr = PetscFree(col);CHKERRQ(ierr);
  ierr = PetscFree(Iidx);CHKERRQ(ierr);
  *W = defl;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "KSPDCGGetDeflationSpaceJacketHaar"
PetscErrorCode KSPDCGGetDeflationSpaceJacketHaar(KSP ksp,Mat *W,PetscInt size)
{
  PetscErrorCode ierr;
  Mat A,*H,defl;
  PetscInt i,m,M,Mdefl,Ndefl;
  PetscBool jh;
  MPI_Comm comm;

  PetscFunctionBegin;
  ierr = PetscObjectGetComm((PetscObject)ksp,&comm);CHKERRQ(ierr);
  ierr = PetscMalloc1(size,&H);CHKERRQ(ierr);
  ierr = KSPGetOperators(ksp,&A,NULL);CHKERRQ(ierr); /* NOTE: Get Pmat instead? */
  ierr = MatGetLocalSize(A,&m,NULL);CHKERRQ(ierr);
  ierr = MatGetSize(A,&M,NULL);CHKERRQ(ierr);
  Mdefl = M;
  Ndefl = M;
  for (i=0; i<size; i++) {
    if (Mdefl%2)  {
      jh=PETSC_TRUE;
      Mdefl = Mdefl/2 +1;
    } else {
      jh=PETSC_FALSE;
      Mdefl = Mdefl/2;
    }
    ierr = KSPDCGDeflationSpaceCreateJacketHaar(comm,PETSC_DECIDE,m,Mdefl,Ndefl,jh,&H[i]);CHKERRQ(ierr);
    ierr = MatGetLocalSize(H[i],&m,NULL);CHKERRQ(ierr);
    Ndefl = Mdefl;
  }
  //ierr = MatCreateProd(comm,size,H,&defl);CHKERRQ(ierr);
  //ierr = MatCreateComposite(comm,size,H,&defl);CHKERRQ(ierr);
  //ierr = MatCompositeSetType(defl,MAT_COMPOSITE_MULTIPLICATIVE);CHKERRQ(ierr);
  /* TODO allow implicit */
  //ierr = MatCompositeMerge(defl);CHKERRQ(ierr);
  Mat newmat;
  defl = H[0];
  for (i=0; i<size-1; i++) {
    ierr = MatMatMult(H[i+1],defl,MAT_INITIAL_MATRIX,PETSC_DECIDE,&newmat);CHKERRQ(ierr);
    ierr = MatDestroy(&defl);CHKERRQ(ierr);
    defl = newmat ;
  }

  ierr = MatTranspose(defl,MAT_INITIAL_MATRIX,W);CHKERRQ(ierr);
  
  ierr = MatDestroy(&defl);CHKERRQ(ierr);
  for (i=1; i<size; i++) {
    ierr = MatDestroy(&H[i]);CHKERRQ(ierr);
  }
  ierr = PetscFree(H);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "KSPDCGGetDeflationSpaceWave"
PetscErrorCode KSPDCGGetDeflationSpaceWave(KSP ksp,Mat *W,PetscInt size,PetscInt ncoeffs,PetscScalar *coeffs,PetscBool trunc)
{
  PetscErrorCode ierr;
  Mat A,*H,defl;
  PetscInt i,m,M,Mdefl,Ndefl;
  MPI_Comm comm;

  PetscFunctionBegin;
  ierr = PetscObjectGetComm((PetscObject)ksp,&comm);CHKERRQ(ierr);
  ierr = PetscMalloc1(size,&H);CHKERRQ(ierr);
  ierr = KSPGetOperators(ksp,&A,NULL);CHKERRQ(ierr); /* NOTE: Get Pmat instead? */
  ierr = MatGetLocalSize(A,&m,NULL);CHKERRQ(ierr);
  ierr = MatGetSize(A,&M,NULL);CHKERRQ(ierr);
  Mdefl = M;
  Ndefl = M;
  for (i=0; i<size; i++) {
    if (Mdefl%2)  {
      if (trunc) {
        Mdefl = (PetscInt)PetscCeilReal(Mdefl/2.);
      } else {
        Mdefl = (PetscInt)PetscFloorReal((ncoeffs+Mdefl-1)/2.);
      }
    } else {
      Mdefl = Mdefl/2;
    }
    ierr = KSPDCGDeflationSpaceCreateWave(comm,PETSC_DECIDE,m,Mdefl,Ndefl,ncoeffs,coeffs,trunc,&H[i]);CHKERRQ(ierr);
    ierr = MatGetLocalSize(H[i],&m,NULL);CHKERRQ(ierr);
    Ndefl = Mdefl;
  }
  ierr = MatCreateProd(comm,size,H,&defl);CHKERRQ(ierr);
  *W = defl;

  for (i=0; i<size; i++) {
    ierr = MatDestroy(&H[i]);CHKERRQ(ierr);
  }
  ierr = PetscFree(H);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "KSPDCGGetDeflationSpaceAggregation"
PetscErrorCode KSPDCGGetDeflationSpaceAggregation(KSP ksp,Mat *W)
{
  PetscErrorCode ierr;
  Mat A,defl;
  PetscInt i,ilo,ihi,*Iidx,m,M;
  PetscReal *col;
  MPI_Comm comm;

  PetscFunctionBegin;
  /* TODO pass A instead of KSP? */
  ierr = KSPGetOperators(ksp,&A,NULL);CHKERRQ(ierr); /* NOTE: Get Pmat instead? */
  ierr = MatGetOwnershipRangeColumn(A,&ilo,&ihi);CHKERRQ(ierr);
  ierr = MatGetSize(A,&M,NULL);CHKERRQ(ierr);
  ierr = PetscObjectGetComm((PetscObject)A,&comm);CHKERRQ(ierr);
  ierr = MPI_Comm_size(comm,&m);CHKERRQ(ierr);
  ierr = MatCreate(comm,&defl);CHKERRQ(ierr);
  ierr = MatSetSizes(defl,ihi-ilo,1,M,m);CHKERRQ(ierr);
  ierr = MatSetUp(defl);CHKERRQ(ierr);
  ierr = MatSeqAIJSetPreallocation(defl,1,NULL);CHKERRQ(ierr);
  ierr = MatMPIAIJSetPreallocation(defl,1,NULL,0,NULL);CHKERRQ(ierr);
  ierr = MatSetOption(defl,MAT_NEW_NONZERO_ALLOCATION_ERR,PETSC_TRUE);CHKERRQ(ierr);
  ierr = MatSetOption(defl,MAT_IGNORE_OFF_PROC_ENTRIES,PETSC_TRUE);CHKERRQ(ierr);
  

  ierr = PetscMalloc1(ihi-ilo,&col);CHKERRQ(ierr);
  ierr = PetscMalloc1(ihi-ilo,&Iidx);CHKERRQ(ierr);
  for (i=ilo; i<ihi; i++) {
    Iidx[i-ilo] = i;
    col[i-ilo] = 1;
  }
  ierr = MPI_Comm_rank(comm,&i);CHKERRQ(ierr);
  ierr = MatSetValues(defl,ihi-ilo,Iidx,1,&i,col,INSERT_VALUES);CHKERRQ(ierr);
    
  ierr = MatAssemblyBegin(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(defl,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  
  ierr = PetscFree(col);CHKERRQ(ierr);
  ierr = PetscFree(Iidx);CHKERRQ(ierr);
  *W = defl;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "KSPDCGGetDeflationSpaceSLEPc"
PetscErrorCode KSPDCGGetDeflationSpaceSLEPc(KSP ksp,Mat *W,PetscInt size,PetscBool cheapCP)
{
#if defined(HAVE_SLEPC)
  PetscErrorCode ierr;
  KSP_DCG *cg = (KSP_DCG*)ksp->data;
  Mat A,defl;
  Vec vec;
  EPS eps;
  PetscScalar *data,*dataScaled,eigval;
  PetscInt i,nconv,m,M,n=PETSC_DECIDE;
  PetscBool slepcinit;
  MPI_Comm comm;

  PetscFunctionBegin;
  ierr = SlepcInitialized(&slepcinit);CHKERRQ(ierr);
  if (!slepcinit) {
    ierr = SlepcInitialize(NULL,NULL,(char*)0,(char*)0);CHKERRQ(ierr);
    slepcinit = PETSC_TRUE;
  }
  ierr = KSPGetOperators(ksp,&A,NULL);CHKERRQ(ierr); /* NOTE: Get Pmat instead? */
  ierr = PetscObjectGetComm((PetscObject)ksp,&comm);CHKERRQ(ierr);
  ierr = EPSCreate(comm,&eps);CHKERRQ(ierr);
  ierr = EPSSetOperators(eps,A,NULL);CHKERRQ(ierr);
  ierr = EPSSetProblemType(eps,EPS_HEP);CHKERRQ(ierr); /* Implemented only for CG */
  ierr = EPSSetWhichEigenpairs(eps,EPS_SMALLEST_REAL);CHKERRQ(ierr);
  ierr = EPSSetDimensions(eps,size,PETSC_DEFAULT,PETSC_DEFAULT);CHKERRQ(ierr);
  ierr = EPSSetFromOptions(eps);CHKERRQ(ierr);

  ierr = EPSSolve(eps);CHKERRQ(ierr);
  ierr = EPSGetConverged(eps,&nconv);CHKERRQ(ierr);
  if (!nconv) SETERRQ(comm,PETSC_ERR_CONV_FAILED,"SLEPc: Number of converged eigenpairs is 0");
  ierr = MatCreateVecs(A,NULL,&vec);CHKERRQ(ierr);
  ierr = MatGetSize(A,&M,NULL);CHKERRQ(ierr);
  ierr = MatGetLocalSize(A,&m,NULL);CHKERRQ(ierr);
  ierr = PetscSplitOwnership(comm,&n,&nconv);CHKERRQ(ierr);
  ierr = PetscMalloc1(m*nconv,&data);CHKERRQ(ierr);
  /* TODO check that eigenvalue is not 0 -> vec is not in Ker A */
  for (i=0; i<nconv; i++) {
    ierr = VecPlaceArray(vec,&data[i*m]);CHKERRQ(ierr);
    ierr = EPSGetEigenvector(eps,i,vec,NULL);CHKERRQ(ierr);
    ierr = VecResetArray(vec);CHKERRQ(ierr);
  }
  ierr = MatCreateDense(comm,m,n,M,nconv,data,&defl);CHKERRQ(ierr);

  if (cheapCP) {
    ierr = PetscMalloc1(m*nconv,&dataScaled);CHKERRQ(ierr);
    for (i=0; i<nconv; i++) {
        ierr = VecPlaceArray(vec,&dataScaled[i*m]);CHKERRQ(ierr);
        ierr = EPSGetEigenpair(eps,i,&eigval,NULL,vec,NULL);CHKERRQ(ierr);
        ierr = VecScale(vec,eigval);CHKERRQ(ierr);
        ierr = VecResetArray(vec);CHKERRQ(ierr);
    }
    ierr = MatCreateDense(comm,m,n,M,nconv,dataScaled,&cg->AW);CHKERRQ(ierr);
  }

  //ierr = EPSGetBV(eps,&bv);CHKERRQ(ierr);
  //ierr = BVCreateMat(bv,&defl);CHKERRQ(ierr);
  *W = defl;

  ierr = EPSDestroy(&eps);CHKERRQ(ierr);
  if (slepcinit) ierr = SlepcFinalize();CHKERRQ(ierr);
  PetscFunctionReturn(0);
#else
  SETERRQ(PetscObjectComm((PetscObject)ksp),PETSC_ERR_CONV_FAILED,"Not compiled with SLEPc support (call make HAVE_SLEPC)");
#endif
}

#undef __FUNCT__
#define __FUNCT__ "KSPDCGComputeDeflationSpace"
PetscErrorCode KSPDCGComputeDeflationSpace(KSP ksp)
{
  PetscErrorCode ierr;
  Mat defl;
  PetscInt size=0;
  PetscBool transp=PETSC_TRUE;
  KSP_DCG *cg = (KSP_DCG*)ksp->data;

  /* TODO valid header */
  PetscFunctionBegin;
  if (cg->spacesize < 1) SETERRQ1(PetscObjectComm((PetscObject)ksp),PETSC_ERR_ARG_WRONG,"Wrong DCG Space size specified: %d",cg->spacesize);
  switch (cg->spacetype) {
    case DCG_SPACE_HAAR:
      ierr = KSPDCGGetDeflationSpaceHaar(ksp,&defl,cg->spacesize);CHKERRQ(ierr);break;
      transp = PETSC_FALSE;
      size = 1;
    case DCG_SPACE_JACKET_HAAR:
      ierr = KSPDCGGetDeflationSpaceJacketHaar(ksp,&defl,cg->spacesize);CHKERRQ(ierr);break;
      transp = PETSC_FALSE;
      size = 1;
    case DCG_SPACE_DB4:
      ierr = KSPDCGGetDeflationSpaceWave(ksp,&defl,cg->spacesize,4,db4,!cg->extendsp);CHKERRQ(ierr);break;
    case DCG_SPACE_DB8:
      ierr = KSPDCGGetDeflationSpaceWave(ksp,&defl,cg->spacesize,8,db8,!cg->extendsp);CHKERRQ(ierr);break;
    case DCG_SPACE_DB16:
      ierr = KSPDCGGetDeflationSpaceWave(ksp,&defl,cg->spacesize,16,db16,!cg->extendsp);CHKERRQ(ierr);break;
    case DCG_SPACE_BIORTH22:
      ierr = KSPDCGGetDeflationSpaceWave(ksp,&defl,cg->spacesize,6,biorth22,!cg->extendsp);CHKERRQ(ierr);break;
    case DCG_SPACE_MEYER:
      ierr = KSPDCGGetDeflationSpaceWave(ksp,&defl,cg->spacesize,62,meyer,!cg->extendsp);CHKERRQ(ierr);break;
    case DCG_SPACE_AGGREGATION:
      ierr = KSPDCGGetDeflationSpaceAggregation(ksp,&defl);CHKERRQ(ierr);break;
      transp = PETSC_FALSE;
      size = 1;
    case DCG_SPACE_SLEPC:
      ierr = KSPDCGGetDeflationSpaceSLEPc(ksp,&defl,cg->spacesize,PETSC_FALSE);CHKERRQ(ierr);break;
      transp = PETSC_FALSE;
      size = 1;
    case DCG_SPACE_SLEPC_CHEAP:
      ierr = KSPDCGGetDeflationSpaceSLEPc(ksp,&defl,cg->spacesize,PETSC_TRUE);CHKERRQ(ierr);break;
      transp = PETSC_FALSE;
      size = 1;
    default: SETERRQ(PetscObjectComm((PetscObject)ksp),PETSC_ERR_ARG_WRONG,"Wrong DCG Space Type specified");
  }
  
  if (!size) size = cg->spacesize;
  ierr = KSPDCGSetDeflationSpace(ksp,defl,transp,size);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}




