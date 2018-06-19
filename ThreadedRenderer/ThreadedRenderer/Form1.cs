using System;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;

namespace ThreadedRenderer
{
    public partial class Form1 : Form
    {
        private DirectBitmap bmp;
        private DirectBitmap membmp;
        private Graphics g;
        private System.Windows.Forms.Timer updateTimer;
        private delegate void PixelFunc(int x, int y, Color c);
        public Form1()
        {
            InitializeComponent();
        }

        public void SetPixel(int x, int y, System.Drawing.Color col)
        {
            lock (membmp)
            {
                membmp.SetPixel(x, y, col);
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            if (bmp != null)
                e.Graphics.DrawImage(bmp.Bitmap, 0, 0, ClientSize.Width, ClientSize.Height);
        }

        private void Init()
        {
            if (g != null) g.Dispose();
            if (bmp != null) bmp.Dispose();
            bmp = new DirectBitmap(ClientSize.Width, ClientSize.Height);
            if (membmp != null) membmp.Dispose();
            membmp = new DirectBitmap(ClientSize.Width, ClientSize.Height);
            //this.BackgroundImage = bmp.Bitmap;
            g = Graphics.FromImage(bmp.Bitmap);
        }

        private void Render(Object state)
        {
            for (int x = 0; x < 320; x++)
                for (int y = 0; y < 240; y++)
                {
                    //Thread.Sleep(1);
                    SetPixel(x, y, Color.FromArgb(x % 255, y % 255, 0));
                }
        }

        private void Button1_Click(object sender, System.EventArgs e)
        {
            Init();

            if (updateTimer == null)
            {
                updateTimer = new System.Windows.Forms.Timer();
                updateTimer.Interval = 100;
                updateTimer.Tick += new EventHandler(UpdateTimer_Tick);
                updateTimer.Start();
            }

            WaitCallback wc = new WaitCallback(Render);
            ThreadPool.QueueUserWorkItem(wc);
        }

        private void UpdateTimer_Tick(object sender, EventArgs e)
        {
            lock (membmp)
            {
                g.DrawImageUnscaled(membmp.Bitmap, 0, 0);
                //g.DrawImage(membmp.Bitmap, 0, 0, ClientSize.Width, ClientSize.Height);
                this.Refresh();
            }
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            g.Dispose();
            bmp.Dispose();
            membmp.Dispose();
        }
    }
}
