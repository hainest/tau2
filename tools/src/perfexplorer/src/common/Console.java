
package common;

    import java.awt.*;
    import java.awt.event.*;
    import java.io.*;
    import javax.swing.*;
    import javax.swing.event.*;
    import javax.swing.text.*;
    
    public class Console extends JFrame {
        JTextPane textPane = new JTextPane();
		private PrintStream out = null;
		private PrintStream err = null;
		private PrintStream oldOut = null;
		private PrintStream oldErr = null;
		private Document doc = null;
		private SimpleAttributeSet errorStyle = null;
		private SimpleAttributeSet outputStyle = null;
    
        public Console() throws IOException {
			super("PerfExplorer Console");

			// preserve the old out and err
			oldOut = System.out;
			oldErr = System.err;

            // Set up System.out
            ConsoleOutputStream consoleOut = new ConsoleOutputStream(this, false);
            out = new PrintStream(consoleOut, true);
            System.setOut(out);
    
            // Set up System.err
            ConsoleOutputStream consoleErr = new ConsoleOutputStream(this, true);
            err = new PrintStream(consoleErr, true);
            System.setErr(err);
    
            // Add a scrolling text area
            textPane.setEditable(false);
            this.getContentPane().add(new JScrollPane(textPane), BorderLayout.CENTER);
			this.setPreferredSize(new Dimension(800,600));
            this.pack();
            this.setVisible(true);

			doc = textPane.getDocument();
			errorStyle = new SimpleAttributeSet();
			StyleConstants.setForeground(errorStyle, Color.red);
			outputStyle = new SimpleAttributeSet();
			StyleConstants.setForeground(outputStyle, Color.black);
        }
    
		public void print(boolean error, String record) {
			AttributeSet attributes = outputStyle;
			if (error) {
				attributes = errorStyle;
			}
			try {
				doc.insertString(doc.getLength(), record, attributes);
			} catch (BadLocationException exp) {
		        exp.printStackTrace();
			}

			// Make sure the last line is always visible
			textPane.setCaretPosition(textPane.getDocument().getLength());
    
		}
    }