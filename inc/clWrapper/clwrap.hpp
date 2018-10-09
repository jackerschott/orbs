#ifndef CLWRAP_HPP
#define CLWRAP_HPP

#include <CL/cl.hpp>

namespace cl {

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19, T20 arg20) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
		kernel.setArg<T12>(11, arg12);
		kernel.setArg<T13>(12, arg13);
		kernel.setArg<T14>(13, arg14);
		kernel.setArg<T15>(14, arg15);
		kernel.setArg<T16>(15, arg16);
		kernel.setArg<T17>(16, arg17);
		kernel.setArg<T18>(17, arg18);
		kernel.setArg<T19>(18, arg19);
		kernel.setArg<T20>(19, arg20);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
		kernel.setArg<T12>(11, arg12);
		kernel.setArg<T13>(12, arg13);
		kernel.setArg<T14>(13, arg14);
		kernel.setArg<T15>(14, arg15);
		kernel.setArg<T16>(15, arg16);
		kernel.setArg<T17>(16, arg17);
		kernel.setArg<T18>(17, arg18);
		kernel.setArg<T19>(18, arg19);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
		kernel.setArg<T12>(11, arg12);
		kernel.setArg<T13>(12, arg13);
		kernel.setArg<T14>(13, arg14);
		kernel.setArg<T15>(14, arg15);
		kernel.setArg<T16>(15, arg16);
		kernel.setArg<T17>(16, arg17);
		kernel.setArg<T18>(17, arg18);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
		kernel.setArg<T12>(11, arg12);
		kernel.setArg<T13>(12, arg13);
		kernel.setArg<T14>(13, arg14);
		kernel.setArg<T15>(14, arg15);
		kernel.setArg<T16>(15, arg16);
		kernel.setArg<T17>(16, arg17);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
		kernel.setArg<T12>(11, arg12);
		kernel.setArg<T13>(12, arg13);
		kernel.setArg<T14>(13, arg14);
		kernel.setArg<T15>(14, arg15);
		kernel.setArg<T16>(15, arg16);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11, typename T12, typename T13, typename T14, typename T15>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
		kernel.setArg<T12>(11, arg12);
		kernel.setArg<T13>(12, arg13);
		kernel.setArg<T14>(13, arg14);
		kernel.setArg<T15>(14, arg15);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11, typename T12, typename T13, typename T14>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11, T12 arg12, T13 arg13, T14 arg14) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
		kernel.setArg<T12>(11, arg12);
		kernel.setArg<T13>(12, arg13);
		kernel.setArg<T14>(13, arg14);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11, typename T12, typename T13>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11, T12 arg12, T13 arg13) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
		kernel.setArg<T12>(11, arg12);
		kernel.setArg<T13>(12, arg13);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11, typename T12>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11, T12 arg12) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
		kernel.setArg<T12>(11, arg12);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10,
		typename T11>
		void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
			T11 arg11) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
		kernel.setArg<T11>(10, arg11);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
	void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
		kernel.setArg<T10>(9, arg10);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
	void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
		kernel.setArg<T9>(8, arg9);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
		kernel.setArg<T8>(7, arg8);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
		kernel.setArg<T7>(6, arg7);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
		kernel.setArg<T6>(5, arg6);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
		kernel.setArg<T5>(4, arg5);
	}

	template<typename T1, typename T2, typename T3, typename T4>
	void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3, T4 arg4) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
		kernel.setArg<T4>(3, arg4);
	}

	template<typename T1, typename T2, typename T3>
	void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2, T3 arg3) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
		kernel.setArg<T3>(2, arg3);
	}

	template<typename T1, typename T2>
	void setKernelArgs(cl::Kernel kernel, T1 arg1, T2 arg2) {
		kernel.setArg<T1>(0, arg1);
		kernel.setArg<T2>(1, arg2);
	}

	template<typename T1>
	void setKernelArgs(cl::Kernel kernel, T1 arg1) {
		kernel.setArg<T1>(0, arg1);
	}

	const char* getErrMsg(int err);
}

#endif
