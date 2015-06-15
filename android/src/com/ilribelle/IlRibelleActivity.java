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

import org.qtproject.qt5.android.bindings.QtActivity;
import android.content.Intent;
import android.os.Bundle;

import android.app.Dialog;
import android.util.Log;

public class IlRibelleActivity extends QtActivity {
	@Override
	public void onCreate(Bundle bundle) {
		super.onCreate(bundle);
		MiscNativeBinding.onCreate(this, bundle);
	}
	@Override
	protected void onStart() {
		super.onStart();
		MiscNativeBinding.onStart();
	}
	@Override
	protected void onRestart() {
		super.onRestart();
		MiscNativeBinding.onRestart();
	}
	@Override
	protected void onResume() {
		super.onResume();
		MiscNativeBinding.onResume();
	}
	@Override
	protected void onSaveInstanceState(Bundle outState) {
		MiscNativeBinding.onSaveInstanceState(outState);
		super.onSaveInstanceState(outState);
	}
	@Override
	public void onPause() {
		MiscNativeBinding.onPause();
		super.onPause();
	}
	@Override
	public void onStop() {
		MiscNativeBinding.onStop();
		super.onStop();
	}
	@Override
	public void onDestroy() {
		MiscNativeBinding.onDestroy();
		super.onDestroy();
	}
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		MiscNativeBinding.onActivityResult(requestCode, resultCode, data);
	}
}
