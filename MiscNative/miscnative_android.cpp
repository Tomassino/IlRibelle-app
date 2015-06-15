/******************************************************************************
 * IlRibelle.com                                                              *
 * Copyright (C) 2014                                                         *
 * Tomassino Ferrauto <t_ferrauto@yahoo.it>                                   *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#include "MiscNative/miscnative.h"
#include <QtAndroidExtras>
#include <QCoreApplication>
#include <QDebug>
#include "include/utilities.h"

namespace MiscNative_internal
{
	// This helper class only contains static functions that are called by
	// java native methods and in turn call MiscNative functions
	class MiscNativeImpl
	{
	public:
		static void actionCompleted()
		{
			// We need to post an event so that the function is executed in the correct thread
			auto command = [](){ MiscNative::instance()->actionCompleted(); };
			QCoreApplication::postEvent(MiscNative::instance(), new CommandEvent(std::move(command)));
		}

		static void actionError(QString reason)
		{
			// We need to post an event so that the function is executed in the correct thread
			auto command = [reason](){ MiscNative::instance()->actionError(reason); };
			QCoreApplication::postEvent(MiscNative::instance(), new CommandEvent(std::move(command)));
		}
	};

	// The name of the Java class
	constexpr const char* javaClassName = "com/ilribelle/MiscNativeBinding";
}

void MiscNative::share(QString link, QString subject, QString shareCaption)
{
	// If we are already doing something, doing nothing
	if (m_status == Acting) {
		return;
	}

	// Setting the status to Acting
	setStatus(Acting);

	// Calling native method
	QAndroidJniObject::callStaticMethod<void>(MiscNative_internal::javaClassName, "share",
		"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
		QAndroidJniObject::fromString(link).object<jstring>(),
		QAndroidJniObject::fromString(subject).object<jstring>(),
		QAndroidJniObject::fromString(shareCaption).object<jstring>());

	// Checking exceptions
	QAndroidJniEnvironment env;
	if (env->ExceptionCheck()) {
		// Printing exception message
		env->ExceptionDescribe();

		// Clearing exceptions
		env->ExceptionClear();
	}
}

void fromJavaActionCompleted(JNIEnv* env, jobject thiz)
{
	Q_UNUSED(env)
	Q_UNUSED(thiz)
	MiscNative_internal::MiscNativeImpl::actionCompleted();
}

void fromJavaActionError(JNIEnv* env, jobject thiz, jstring reason)
{
	Q_UNUSED(env)
	Q_UNUSED(thiz)
	MiscNative_internal::MiscNativeImpl::actionError(QAndroidJniObject(reason).toString());
}

static JNINativeMethod methods[] {
	{"actionCompleted", "()V", (void*)(fromJavaActionCompleted)},
	{"actionError", "(Ljava/lang/String;)V", (void*)(fromJavaActionError)}
};

#ifdef MISC_NATIVE_NO_JNI_ONLOAD_DEFINITION
int miscNative_registerJavaNativeMethods(JavaVM* vm, void*) {
#else
extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
#endif
	JNIEnv *env;
	if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4) != JNI_OK) {
		return JNI_FALSE;
	}
	jclass clazz = env->FindClass(MiscNative_internal::javaClassName);
	if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
		return JNI_FALSE;
	}
	return JNI_VERSION_1_4;
}
