      subroutine sizeof_int (bits)
      integer bits
      integer i,j

      i=0
      j=1
 10   i=i+1
      j=j+j
      if (j.ne.0) go to 10
      bits = i / 8
      end


      program hello
      integer bits

      call sizeof_int (bits)
      write (*,*) 'Size of integer: ', bits

      end program hello
