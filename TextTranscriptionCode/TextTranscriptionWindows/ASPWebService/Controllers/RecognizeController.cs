using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.Web.Http;
using TextTranscriptionCLRComp;
using System.Web.Routing;
using System.Collections.Specialized;
using System.Net.Http;
using Newtonsoft.Json.Linq;
using System.Threading.Tasks;
using System.Threading;

namespace ASPWebService.Controllers
{
    public class RecognizeController : ApiController
    {
		public class RecoRequest
		{
			public String base64image;
		}

		private static string filePath;
		private static byte[] curBytesToConvert;
		private static String recoResult;
		private static Semaphore transcriptionStartSem = new Semaphore(0, 1);
		private static Semaphore transcriptionEndSem = new Semaphore(0, 1);

		private static Thread transcriptionThread = new Thread(() =>
		{
			TextTranscriptionService transcriptionService = new TextTranscriptionService(filePath);
			while (true)
			{
				transcriptionStartSem.WaitOne();
				recoResult = transcriptionService.recognizeImage(curBytesToConvert);
				transcriptionEndSem.Release();
			}
		}, 4194304);

		public String Post([FromBody]RecoRequest recoRequest)
		{
			HttpContextBase httpContext = new HttpContextWrapper(HttpContext.Current);
			var urlResource = UrlHelper.GenerateContentUrl("~/Content/ResNetCRNNNewCNTK32SeqLenOut", httpContext);

			filePath = System.Web.Hosting.HostingEnvironment.MapPath(urlResource);
			curBytesToConvert = System.Convert.FromBase64String(recoRequest.base64image);

			lock (transcriptionThread)
			{
				if (transcriptionThread.ThreadState == ThreadState.Unstarted)
				{
					transcriptionThread.Start();
				}
				transcriptionStartSem.Release();
				transcriptionEndSem.WaitOne();
			}

			return recoResult;
		}
	}
}