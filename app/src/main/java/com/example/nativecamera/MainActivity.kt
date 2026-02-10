package com.example.nativecamera

import android.Manifest
import android.content.pm.PackageManager
import android.graphics.SurfaceTexture
import android.os.Bundle
import android.view.Surface
import android.view.TextureView
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat

class MainActivity : AppCompatActivity() {

    external fun nativeSetSurface(surface: Surface)
    external fun nativeOnPermissionResult(granted: Boolean)
    external fun nativeOnResume()
    external fun nativeOnPause()
    external fun nativeOnDestroy()
    external fun nativeCaptureImage()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val textureView = findViewById<TextureView>(R.id.textureView)

        textureView.surfaceTextureListener =
            object : TextureView.SurfaceTextureListener {

                override fun onSurfaceTextureAvailable(
                    surface: SurfaceTexture,
                    width: Int,
                    height: Int
                ) {
                    nativeSetSurface(Surface(surface))
                }

                override fun onSurfaceTextureDestroyed(surface: SurfaceTexture): Boolean {
                    return true
                }

                override fun onSurfaceTextureSizeChanged(
                    surface: SurfaceTexture,
                    width: Int,
                    height: Int
                ) {}

                override fun onSurfaceTextureUpdated(surface: SurfaceTexture) {}
            }

        findViewById<Button>(R.id.btnPhoto).setOnClickListener {
            nativeCaptureImage()
        }

        requestCameraPermission()
    }

    override fun onResume() {
        super.onResume()
        nativeOnResume()
    }

    override fun onPause() {
        nativeOnPause()
        super.onPause()
    }

    override fun onDestroy() {
        nativeOnDestroy()
        super.onDestroy()
    }

    private fun requestCameraPermission() {
        if (ContextCompat.checkSelfPermission(
                this,
                Manifest.permission.CAMERA
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(
                this,
                arrayOf(Manifest.permission.CAMERA),
                100
            )
        } else {
            nativeOnPermissionResult(true)
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        if (requestCode == 100) {
            nativeOnPermissionResult(
                grantResults.isNotEmpty() &&
                        grantResults[0] == PackageManager.PERMISSION_GRANTED
            )
        }
    }

    companion object {
        init {
            System.loadLibrary("nativecamera")
        }
    }
}
