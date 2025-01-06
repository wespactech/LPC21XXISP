package com.xysantai.myisp;

import static android.content.Context.MODE_PRIVATE;

import android.app.Application;
import android.app.Dialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class InputDialog extends Dialog {
    private EditText editText;
    private Button buttonSubmit;



    public InputDialog(Context context) {
        super(context);
        setContentView(R.layout.dialog_input_layout);
        editText = findViewById(R.id.edit_text);
        buttonSubmit = findViewById(R.id.button_submit);
        buttonSubmit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String filename = editText.getText().toString().trim();
                if(!filename.isEmpty()){
                    MyApplication.ispFile = filename ;

                    SharedPreferences sp = context.getSharedPreferences("UpgradeFiles",MODE_PRIVATE);
                    SharedPreferences.Editor editor = sp.edit();
                    //editor.putString("IspFilename", filename);
                   // editor.putString("ApkFilename", filename);
                    editor.putString("IspFilename", "VER_1_A_B286.bin");
                    editor.putString("ApkFilename", "VER_1_500~800WUART40J.apk");
                    editor.commit();


                }
                dismiss();
            }
        });
    }
}
