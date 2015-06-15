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

package com.ilribelle;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ResolveInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.List;

/**
 * Java class acting as a bridge from Java to C++ code for MiscNative.
 *
 * This class is a singleton because it's a binding of a singleton C++ object.
 * All the methods are static, and they use the private singleton instance. For
 * the moment we only have code to share a link using ACTION_SEND intent.
 *
 * To work correctly, the onCreate, onResume, onSaveInstanceState, onPause,
 * onDestroy and onActivityResult methods must be called in the corresponding
 * methods of the activity
 */
public class MiscNativeBinding
{
	/**
	 * A utility static nested class that starts an Intent to share a link.
	 *
	 * This is needed because we need to run the intent in the main activity
	 * thread. It is not inline because I don't like to declare and
	 * instantiate classes in a function call
	 */
	static private class RunShareIntent implements Runnable
	{
		/**
		 * The link to post.
		 */
		private String m_link;

		/**
		 * The subject for the link to share.
		 */
		private String m_subject;

		/**
		 * The caption of the share dialog.
		 */
		private String m_shareCaption;

		/**
		 * The instance of MiscNativeBinding
		 */
		private MiscNativeBinding m_miscNativeBinding;

		/**
		 * Constructor.
		 *
		 * @param link the link to share
		 * @param subject the subject for the link to share
		 * @param shareCaption the caption of the share dialog
		 * @param miscNativeBinding the instance of MiscNativeBinding
		 */
		public RunShareIntent(String link, String subject, String shareCaption, MiscNativeBinding miscNativeBinding)
		{
			m_link = link;
			m_subject = subject;
			m_shareCaption = shareCaption;
			m_miscNativeBinding = miscNativeBinding;
		}

		/**
		 * The function doing the actual work
		 */
		@Override
		public void run()
		{
			// Create intent using ACTION_SEND
			Intent intent = new Intent(Intent.ACTION_SEND);
			intent.setType("text/plain");
			intent.putExtra(Intent.EXTRA_SUBJECT, m_subject);
			intent.putExtra(Intent.EXTRA_TEXT, m_link);
			Intent chooser = Intent.createChooser(intent, m_shareCaption);

			// Verify that the intent will resolve to an activity
			if (intent.resolveActivity(m_miscNativeBinding.m_activity.getPackageManager()) != null) {
				// Launching the share intent
				m_miscNativeBinding.m_shareIntentPosted = true;
				m_miscNativeBinding.m_activity.startActivity(chooser);
			} else {
				// Raising an error
				m_miscNativeBinding.actionError("Cannot post share intent");
				Log.i("MiscNativeBinding", "Share intent posting error");
			}

		}
	}

	/**
	 * Singleton instance created as soon as possibile.
	 */
	private static final MiscNativeBinding m_instance = new MiscNativeBinding();

	/**
	 * The activity to which we are associated.
	 */
	private Activity m_activity = null;

	/**
	 * This is set to true just before posting the intent.
	 */
	private boolean m_shareIntentPosted = false;

	/**
	 * This has to be called inside the onCreate function of the activity.
	 */
	static public void onCreate(Activity activity, Bundle savedInstanceState)
	{
		Log.i("MiscNativeBinding", "onCreate CALLED!");

		m_instance.m_activity = activity;

		// We do not need to do anything else here, for the moment
	}

	/**
	 * This has to be called inside the onStart function of the activity.
	 */
	static public void onStart()
	{
		Log.i("MiscNativeBinding", "onStart CALLED!");
		// We do not need to do anything here, for the moment
	}

	/**
	 * This has to be called inside the onRestart function of the activity.
	 */
	static public void onRestart()
	{
		Log.i("MiscNativeBinding", "onRestart CALLED!");

		// If we were posting a share intent we are getting here after the intent
		// has finished (probably...)
		if (m_instance.m_shareIntentPosted) {
			actionCompleted();
			Log.i("MiscNativeBinding", "Share intent posted!");
		}

		m_instance.m_shareIntentPosted = false;
	}

	/**
	 * This has to be called inside the onResume function of the activity.
	 */
	static public void onResume()
	{
		Log.i("MiscNativeBinding", "onResume CALLED!");
		// We do not need to do anything here, for the moment
	}

	/**
	 * This has to be called inside the onSaveInstanceState function of the
	 * activity.
	 */
	static public void onSaveInstanceState(Bundle outState)
	{
		Log.i("MiscNativeBinding", "onSaveInstanceState CALLED!");
		// We do not need to do anything here, for the moment
	}

	/**
	 * This has to be called inside the onPause function of the activity.
	 */
	static public void onPause()
	{
		Log.i("MiscNativeBinding", "onPause CALLED!");
		// We do not need to do anything here, for the moment
	}

	/**
	 * This has to be called inside the onStop function of the activity.
	 */
	static public void onStop()
	{
		Log.i("MiscNativeBinding", "onStop CALLED!");
		// We do not need to do anything here, for the moment
	}

	/**
	 * This has to be called inside the onDestroy function of the activity.
	 */
	static public void onDestroy()
	{
		Log.i("MiscNativeBinding", "onDestroy CALLED!");
		// We do not need to do anything here, for the moment
	}

	/**
	 * This has to be called inside the onActivityResult function of the
	 * activity.
	 */
	static public void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		Log.i("MiscNativeBinding", "onActivityResult CALLED!");
		// We do not need to do anything here, for the moment
	}

	/**
	 * The function to post a share intent.
	 *
	 * @param link the link to share
	 * @param subject the subject for the link to share
	 * @param shareCaption the caption of the share dialog
	 */
	static public void share(String link, String subject, String shareCaption)
	{
		Log.i("MiscNativeBinding", link);

		RunShareIntent r = new RunShareIntent(link, subject, shareCaption, m_instance);

		// Starting the intent
		m_instance.m_activity.runOnUiThread(r);
	}

	/**
	 * The native function called to signal that a share intent was posted.
	 *
	 * Implementation is in miscnative_android.cpp
	 */
	static private native void actionCompleted();

	/**
	 * The native function called to signal that an error occurred when
	 * posting a share intent.
	 *
	 * Implementation is in miscnative_android.cpp
	 */
	static private native void actionError(String reason);
}
